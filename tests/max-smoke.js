autowatch = 1;
inlets = 3;
outlets = 0;
var target, ui, steps, tick = 0;
var classname = jsarguments.length > 1 ? jsarguments[1] : "bbb.opendmxusb";
var results = [];
var patchdir = this.patcher.filepath.slice(0, this.patcher.filepath.lastIndexOf("/"));
var output = patchdir.slice(0, patchdir.lastIndexOf("/")) + "/artifacts/max-smoke.json";
function record(name, actual, expected) {
    var ok = JSON.stringify(actual) === JSON.stringify(expected);
    results.push({name:name, actual:actual, expected:expected, passed:ok});
}
function list() {
    if (inlet === 1) record("dump", arrayfromargs(arguments), [255, 33, 44, 55]);
}
function error() { results.push({event:"error", args:arrayfromargs(arguments)}); }
function anything() {
    if (inlet === 2) results.push({event:messagename, args:arrayfromargs(arguments)});
}
function run() {
    target = this.patcher.newdefault(40, 150, classname, "@mode", 1, "@num_channels", 4);
    ui = this.patcher.newdefault(40, 100, "attrui", "@attr", "port");
    ui.rect = [40, 100, 540, 122];
    this.patcher.connect(ui, 0, target, 0);
    this.patcher.connect(target, 0, this.box, 1);
    this.patcher.connect(target, 1, this.box, 2);
    steps = new Task(step, this); steps.interval = 100; steps.repeat();
}
function step() {
    try {
        if (tick === 0) {
            record("loaded", target.maxclass, classname);
            record("mode", target.getattr("mode"), 1);
            record("num_channels", target.getattr("num_channels"), 4);
            record("port", target.getattr("port"), "none");
            target.message("ports");
            target.message("list", 300, -4, 12.8, 10);
            target.message("setchannel", 2, 33);
            target.message("set_offset", 3, 44, 55);
            target.message("blackout", 1);
            target.message("dump");
        } else if (tick === 1) {
            target.message("origin", 0);
            record("origin", target.getattr("origin"), 0);
            record("blackout", target.getattr("blackout"), 1);
            target.message("status");
            target.message("port", "/dev/cu.bbb-opendmxusb-missing-test-port");
        } else if (tick === 3) {
            target.message("status");
            target.message("port", "none");
        } else if (tick === 4) {
            var f = new File(output, "write", "TEXT"); f.eof = 0; f.writestring(JSON.stringify(results, null, 2)); f.close();
            steps.cancel();
            post("opendmx smoke results: " + output + "\n");
        }
        tick++;
    } catch (e) {
        results.push({exception:String(e)});
        var f = new File(output, "write", "TEXT"); f.eof = 0; f.writestring(JSON.stringify(results, null, 2)); f.close();
        steps.cancel();
    }
}
