#include "ext.h"
#include "ext_obex.h"
#include "engine.hpp"
#include "serial_mac.hpp"
#include <os/log.h>
#include <cmath>
#include <cstdio>
#include <new>
using Engine = opendmx::Engine<opendmx::Serial>;
struct OpenDMX {
    t_object object;
    void* output;
    void* status;
    t_qelem* delivery;
    Engine* engine;
    t_symbol* port;
    t_atom_long mode, num_channels, origin, blackout, verbose;
    double framerate;
    bool initialized;
};
static t_class* klass;
static os_log_t logger;
static void configure(OpenDMX* x) {
    if (x->engine) x->engine->configure(static_cast<int>(x->mode), static_cast<int>(x->num_channels), x->blackout != 0, x->framerate, static_cast<int>(x->origin));
}
static void status_symbol(OpenDMX* x, const char* name, const std::string& text) {
    t_atom a; atom_setsym(&a, gensym(text.c_str()));
    outlet_anything(x->status, gensym(name), text.empty() ? 0 : 1, &a);
}
static void delivery(OpenDMX* x) {
    auto report = x->engine->report();
    for (const auto& event : report.events) {
        os_log(logger, "%{public}s %{public}s", event.kind.c_str(), event.detail.c_str());
        if (event.kind == "error") object_error(reinterpret_cast<t_object*>(x), "%s", event.detail.c_str());
        else if (x->verbose) object_post(reinterpret_cast<t_object*>(x), "%s %s", event.kind.c_str(), event.detail.c_str());
        status_symbol(x, event.kind.c_str(), event.detail);
    }
    // Bound catch-up work if Max's low-priority queue was blocked for a long time.
    if (report.sent > 256) {
        t_atom a; atom_setlong(&a, static_cast<t_atom_long>(report.sent - 256));
        outlet_anything(x->status, gensym("coalesced"), 1, &a);
    }
    for (uint64_t i = 0; i < std::min<uint64_t>(report.sent, 256); ++i) outlet_bang(x->output);
}
static void refresh_ports(OpenDMX* x, bool output) {
    auto ports = opendmx::serial_ports();
    std::vector<t_atom> choices(ports.size() + 1);
    atom_setsym(&choices[0], gensym("none"));
    for (size_t i = 0; i < ports.size(); ++i) atom_setsym(&choices[i + 1], gensym(ports[i].c_str()));
    // Use the SDK's attribute-of-attribute API so each object's inspector updates.
    const auto err = object_attr_attr_setvalueof(reinterpret_cast<t_object*>(x), gensym("port"), gensym("enumvals"), static_cast<long>(choices.size()), choices.data());
    if (err) object_error(reinterpret_cast<t_object*>(x), "could not update port menu (%ld)", static_cast<long>(err));
    object_attr_touch(reinterpret_cast<t_object*>(x), gensym("port"));
    if (output) outlet_anything(x->status, gensym("ports"), static_cast<short>(ports.size()), choices.data() + 1);
}
static void ports_low(OpenDMX* x, t_symbol*, long, t_atom*) { refresh_ports(x, true); }
static void ports(OpenDMX* x) { defer_low(x, reinterpret_cast<method>(ports_low), nullptr, 0, nullptr); }
static void set_attribute_low(OpenDMX* x, t_symbol* name, long ac, t_atom* av) {
    object_attr_setvalueof(x, name, ac, av);
}
static t_max_err set_port(OpenDMX* x, void*, long ac, t_atom* av) {
    if (ac != 1 || atom_gettype(av) != A_SYM) return MAX_ERR_GENERIC;
    auto* port = atom_getsym(av);
    const std::string path = port->s_name;
    if (path != "none" && path.rfind("/dev/cu.", 0) != 0) {
        object_error(reinterpret_cast<t_object*>(x), "port must be none or a /dev/cu.* path; send ports to list devices");
        return MAX_ERR_GENERIC;
    }
    if (!systhread_ismainthread()) {
        defer_low(x, reinterpret_cast<method>(set_attribute_low), gensym("port"), ac, av);
        return MAX_ERR_NONE;
    }
    x->port = port;
    if (x->engine && x->initialized) x->engine->connect(path == "none" ? "" : path);
    return MAX_ERR_NONE;
}
static t_max_err set_number(OpenDMX* x, void* attr, long ac, t_atom* av) {
    if (ac != 1 || (atom_gettype(av) != A_LONG && atom_gettype(av) != A_FLOAT)) return MAX_ERR_GENERIC;
    const double v = atom_getfloat(av);
    if (!std::isfinite(v)) return MAX_ERR_GENERIC;
    const auto name = static_cast<t_symbol*>(object_method(attr, gensym("getname")));
    if (!systhread_ismainthread()) {
        defer_low(x, reinterpret_cast<method>(set_attribute_low), name, ac, av);
        return MAX_ERR_NONE;
    }
    if (name == gensym("mode")) x->mode = static_cast<long>(std::clamp(v, 0.0, 4.0));
    else if (name == gensym("num_channels")) x->num_channels = static_cast<long>(std::clamp(v, 1.0, 512.0));
    else if (name == gensym("origin")) x->origin = static_cast<long>(std::clamp(v, 0.0, 1.0));
    else if (name == gensym("blackout")) x->blackout = v != 0;
    else if (name == gensym("verbose")) x->verbose = v != 0;
    else if (name == gensym("framerate")) x->framerate = std::clamp(v, 0.01, 44.0);
    configure(x);
    return MAX_ERR_NONE;
}
static void open_low(OpenDMX* x, t_symbol*, long ac, t_atom* av) {
    if (ac) { object_attr_setvalueof(x, gensym("port"), ac, av); return; }
    if (x->port == gensym("none")) {
        object_error(reinterpret_cast<t_object*>(x), "select a port first (ports / port /dev/cu...)"); return;
    }
    x->engine->connect(x->port->s_name);
}
static void open_port(OpenDMX* x, t_symbol* s, long ac, t_atom* av) { defer_low(x, reinterpret_cast<method>(open_low), s, ac, av); }
static void close_low(OpenDMX* x, t_symbol*, long, t_atom*) { x->engine->connect(""); }
static void close_port(OpenDMX* x) { defer_low(x, reinterpret_cast<method>(close_low), nullptr, 0, nullptr); }
static void status_low(OpenDMX* x, t_symbol*, long, t_atom*) {
    t_atom a[2]; atom_setlong(a, x->engine->connected()); atom_setsym(a + 1, x->port);
    outlet_anything(x->status, gensym("status"), 2, a);
}
static void status(OpenDMX* x) { defer_low(x, reinterpret_cast<method>(status_low), nullptr, 0, nullptr); }
static void data(OpenDMX* x, t_symbol* s, long ac, t_atom* av) {
    size_t offset = 0;
    const bool channel = s == gensym("channel") || s == gensym("setchannel");
    if (channel || s == gensym("set_offset")) {
        if (ac < 2 || (channel && ac != 2) || atom_gettype(av) != A_LONG) {
            object_error(reinterpret_cast<t_object*>(x), "%s requires an integer channel and value(s)", s->s_name); return;
        }
        const auto index = atom_getlong(av);
        const auto origin = x->engine->origin();
        if (index < origin || index > origin + 511) {
            object_error(reinterpret_cast<t_object*>(x), "channel is outside the 512-slot buffer"); return;
        }
        offset = static_cast<size_t>(index - origin); ++av; --ac;
    }
    if (ac <= 0) return;
    std::array<uint8_t, 512> values;
    const long count = std::min(ac, static_cast<long>(512 - offset));
    for (long i = 0; i < count; ++i) {
        if ((atom_gettype(av + i) != A_LONG && atom_gettype(av + i) != A_FLOAT) || !std::isfinite(atom_getfloat(av + i))) {
            object_error(reinterpret_cast<t_object*>(x), "DMX values must be finite numbers"); return;
        }
        values[i] = static_cast<uint8_t>(std::clamp(atom_getfloat(av + i), 0.0, 255.0));
    }
    x->engine->update(offset, values.data(), count, s == gensym("list") || s == gensym("channel"));
}
static void input_int(OpenDMX* x, t_atom_long value) {
    t_atom a; atom_setlong(&a, value);
    data(x, gensym("list"), 1, &a);
}
static void input_float(OpenDMX* x, double value) {
    t_atom a; atom_setfloat(&a, value);
    data(x, gensym("list"), 1, &a);
}
static void bang(OpenDMX* x) { x->engine->bang(); }
static void dump_low(OpenDMX* x, t_symbol*, long, t_atom*) {
    const auto data = x->engine->dump();
    std::vector<t_atom> atoms(data.size());
    for (size_t i = 0; i < data.size(); ++i) atom_setlong(&atoms[i], data[i]);
    outlet_list(x->output, nullptr, static_cast<short>(atoms.size()), atoms.data());
}
static void dump(OpenDMX* x) { defer_low(x, reinterpret_cast<method>(dump_low), nullptr, 0, nullptr); }
static void assist(OpenDMX*, void*, long m, long a, char* text) {
    std::snprintf(text, 512, "%s", m == ASSIST_INLET ? "DMX list / channel / set / bang; port and attributes" :
        (a == 0 ? "Bang after serial frame / dump list" : "ports / opened / closed / error / status"));
}
static void free_object(OpenDMX* x) {
    delete x->engine; // join before destroying its notification target
    if (x->delivery) qelem_free(x->delivery);
}
static void* new_object(t_symbol*, long ac, t_atom* av) {
    auto* x = reinterpret_cast<OpenDMX*>(object_alloc(klass));
    if (!x) return nullptr;
    x->mode = 4; x->num_channels = 512; x->origin = 1; x->framerate = 40;
    x->blackout = 0; x->verbose = 0; x->port = gensym("none");
    x->engine = nullptr; x->delivery = nullptr; x->initialized = false;
    x->status = outlet_new(x, nullptr); x->output = outlet_new(x, nullptr);
    x->delivery = qelem_new(x, reinterpret_cast<method>(delivery));
    if (!x->delivery) { object_free(x); return nullptr; }
    try { x->engine = new Engine([x] { qelem_set(x->delivery); }); }
    catch (const std::exception& error) {
        object_error(reinterpret_cast<t_object*>(x), "worker creation failed: %s", error.what());
        object_free(x); return nullptr;
    }
    refresh_ports(x, false);
    attr_args_process(x, ac, av);
    configure(x);
    x->initialized = true;
    if (x->port != gensym("none")) x->engine->connect(x->port->s_name);
    return x;
}
extern "C" void C74_EXPORT ext_main(void*) {
    logger = os_log_create("jp.2bit.bbb.opendmxusb", "serial");
    auto* c = class_new("bbb.opendmxusb", reinterpret_cast<method>(new_object), reinterpret_cast<method>(free_object), sizeof(OpenDMX), nullptr, A_GIMME, 0);
    klass = c;
    class_addmethod(c, reinterpret_cast<method>(input_int), "int", A_LONG, 0);
    class_addmethod(c, reinterpret_cast<method>(input_float), "float", A_FLOAT, 0);
    class_addmethod(c, reinterpret_cast<method>(data), "list", A_GIMME, 0);
    for (const auto* name : {"channel", "setchannel", "set", "set_offset"}) class_addmethod(c, reinterpret_cast<method>(data), name, A_GIMME, 0);
    class_addmethod(c, reinterpret_cast<method>(bang), "bang", 0);
    class_addmethod(c, reinterpret_cast<method>(dump), "dump", 0);
    class_addmethod(c, reinterpret_cast<method>(ports), "ports", 0);
    class_addmethod(c, reinterpret_cast<method>(ports), "refresh", 0);
    class_addmethod(c, reinterpret_cast<method>(open_port), "open", A_GIMME, 0);
    class_addmethod(c, reinterpret_cast<method>(close_port), "close", 0);
    class_addmethod(c, reinterpret_cast<method>(status), "status", 0);
    class_addmethod(c, reinterpret_cast<method>(assist), "assist", A_CANT, 0);
    CLASS_ATTR_SYM(c, "port", 0, OpenDMX, port);
    CLASS_ATTR_ACCESSORS(c, "port", nullptr, set_port);
    CLASS_ATTR_ENUM(c, "port", 0, "none");
    CLASS_ATTR_LABEL(c, "port", 0, "Serial Port");
    CLASS_ATTR_SAVE(c, "port", 0);
#define DMX_LONG(name) \
    CLASS_ATTR_LONG(c, #name, 0, OpenDMX, name); \
    CLASS_ATTR_ACCESSORS(c, #name, nullptr, set_number); \
    CLASS_ATTR_SAVE(c, #name, 0)
    DMX_LONG(mode); DMX_LONG(num_channels); DMX_LONG(origin); DMX_LONG(blackout); DMX_LONG(verbose);
#undef DMX_LONG
    CLASS_ATTR_ENUMINDEX(c, "mode", 0, "automatic bang update change forced");
    CLASS_ATTR_FILTER_CLIP(c, "num_channels", 1, 512);
    CLASS_ATTR_ENUMINDEX(c, "origin", 0, "0-based 1-based");
    CLASS_ATTR_STYLE(c, "blackout", 0, "onoff");
    CLASS_ATTR_STYLE(c, "verbose", 0, "onoff");
    CLASS_ATTR_DOUBLE(c, "framerate", 0, OpenDMX, framerate);
    CLASS_ATTR_ACCESSORS(c, "framerate", nullptr, set_number);
    CLASS_ATTR_FILTER_CLIP(c, "framerate", 0.01, 44);
    CLASS_ATTR_SAVE(c, "framerate", 0);
    class_register(CLASS_BOX, c);
}
