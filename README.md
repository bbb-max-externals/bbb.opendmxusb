# bbb.opendmxusb

A Max external for sending DMX to an **ENTTEC OPEN DMX USB** on macOS 11 or later. The universal binary runs on Apple Silicon and Intel Macs. A dedicated thread drives the FTDI virtual serial port (VCP) at 250,000 baud, 8N2. Each frame starts with a break and a mark after break (MAB), followed by start code 0 and the channel values.

The external uses the DMX messages from `bbb.artnet.controller` and omits its universe, IP, Art-Net and OSC settings. The USB PRO packet format and RDM are unsupported.

## Installation and first output

1. Place the package in `~/Documents/Max 9/Packages/bbb.opendmxusb`. For development, a symlink to the package directory also works.
2. If you updated the external, save your work and restart Max.
3. Create a `[bbb.opendmxusb]` object and open its help patch.
4. Connect `[attrui @attr port]` to the object and select the `/dev/cu.usbserial-...` port for your ENTTEC interface. The port opens asynchronously.
5. Send `channel 1 255` to set the first channel to full, or send a list such as `255 128 0` to set the first three channels.

The object scans for ports at startup. After plugging in or unplugging a device, send `ports` or `refresh` to update the menu. A new object starts with `port none`. If you save a patch with a selected `@port`, the object reconnects to that port when you reopen the patch. Send `close` to disconnect and keep the selection for a later `open`. Send `port none` to disconnect and clear the selection.

The FTDI interface must appear in macOS as a virtual serial port. A D2XX-only setup cannot provide that port. The menu lists all macOS callout serial ports, including Bluetooth ports. Select the one that belongs to your OPEN DMX USB, since the external cannot identify the device type from the port name.

## Messages

| Message | Action |
| --- | --- |
| `list V...`, a numeric list or a single number | Update values starting at the first channel. The current mode determines whether to request a frame. |
| `channel CH VALUE` | Update one channel. The current mode determines whether to request a frame. |
| `set V...` | Store values starting at the first channel without requesting a frame. |
| `setchannel CH VALUE` | Store one channel value without requesting a frame. |
| `set_offset CH V...` | Store consecutive values starting at channel `CH` without requesting a frame. |
| `bang` | Request a frame with the current values in any mode. |
| `dump` | Send the stored values, before blackout, to the left outlet as a list. |
| `ports` / `refresh` | Rescan the ports, update the `attrui` menu and send `ports PATH...` to the right outlet. |
| `port PATH` / `open PATH` | Select a serial port and open it asynchronously. |
| `open` | Reconnect to the selected port. |
| `close` | Disconnect and keep the selected port and stored DMX values. |
| `status` | Send `status CONNECTED SELECTED_PORT` to the right outlet. |

Channel values are clipped to 0–255 and fractional values are truncated. Nonnumeric values, NaN and invalid starting channels produce an error. The buffer always holds 512 channels. Lists that extend past the end are truncated, and channels outside the updated range keep their values. Reducing `num_channels` also preserves the stored values beyond the new output range.

## Attributes

| Attribute | Default | Meaning |
| --- | --- | --- |
| `port` | `none` | Serial port path, selectable with `attrui`. |
| `num_channels` | `512` | Number of channels to transmit, from 1 to 512. |
| `origin` | `1` | Channel numbering for `channel`, `setchannel` and `set_offset`. Set to 0 for zero-based numbering or 1 for one-based numbering. |
| `blackout` | `0` | Set to 1 to mask stored levels with zeros. While connected, changing blackout requests a frame in modes 0, 2 and 3. Clearing it restores the stored levels. Mode 1 waits for a bang and mode 4 uses the next refresh. |
| `mode` | `4` | Transmission mode, as listed below. |
| `framerate` | `40.` | Maximum frame rate, from 0.01 to 44 fps. |
| `verbose` | `0` | Set to 1 to print connection events to the Max Console. |

| Mode | Name | When it sends |
| --- | --- | --- |
| 0 | automatic | On `list` or `channel`. |
| 1 | bang | Only on `bang`. |
| 2 | update | On `list` or `channel`, matching automatic mode in the reference API. |
| 3 | change | On `list` or `channel` if the outgoing frame differs from the last one requested. |
| 4 | forced | Continuously, even when values stay the same. Stored updates take effect on the next refresh. |

The OPEN DMX USB relies on the host to repeat frames, so this external defaults to **forced mode (4)**. The Art-Net reference implementation defaults to automatic mode (0). In event-driven modes, a gap between messages can cause a fixture to time out.

Transmission is asynchronous in every mode. When requests arrive faster than the port can transmit them, the worker keeps only the latest pending frame. The `set` messages update stored values for future frames. A frame already requested keeps its original values. Switching or closing the port discards pending requests. After a connection error, send `open` to retry.

The actual frame rate depends on the USB driver, OS scheduling and channel count. The host waits at least 200 µs for the break and 20 µs for the MAB. Once the output queue drains, the worker waits one frame time plus 1 ms to allow the UART to finish transmitting. Verify electrical timing with a measurement at the DMX output.

The left outlet sends a bang when the serial send routine finishes and a list in response to `dump`. The bang reports host-side completion. Fixture reception requires a separate check. The right outlet reports `ports`, `opened PATH`, `closed`, `error DESCRIPTION` and `status`. If Max stalls long enough for notifications to accumulate, the object emits up to 256 catch-up bangs and reports the remaining count as `coalesced N`.

## Build

The build requires CMake, Xcode Command Line Tools and the pinned Cycling '74 `max-sdk-base` submodule.

```sh
git submodule update --init
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j 4
```

To use a different SDK checkout, pass `-DMAX_SDK_BASE=/path/to/max-sdk-base` at configure time. The build produces `externals/bbb.opendmxusb.mxo` and applies an ad hoc signature for local use.

## Verification and logs

```sh
cmake -S . -B build -DOPEN_DMX_BUILD_TESTS=ON
cmake --build build -j 4
ctest --test-dir build --output-on-failure
```

The engine tests use a simulated port to check frame boundaries, level retention during blackout, transmission modes, refresh rate changes, disconnection, reconnection and shutdown.

To check the API in Max, create the `artifacts/` directory and open `tests/max-smoke.maxpat`. The patch attempts to open a deliberately nonexistent test port and writes the results to `artifacts/max-smoke.json`. For the recorded results and the remaining hardware checks, see [Verification](docs/verification.md).

Connection events and errors are also recorded in the macOS unified log. DMX channel values are excluded.

```sh
/usr/bin/log show --last 10m --style compact \
  --predicate 'subsystem == "jp.2bit.bbb.opendmxusb"'
```

## References

- Interface specifications, FTDI drivers and developer resources for the [ENTTEC OPEN DMX USB](https://www.enttec.com/product/dmx-usb-interfaces/open-dmx-usb/).
- USB serial device and virtual COM port identification in the [ENTTEC datasheet](https://cdn.enttec.com/pdf/assets/70303/70303_OPEN_DMX_USB_DATASHEET.pdf).
- Serial port configuration with termios and IOSSIOSPEED in Apple's [SerialPortSample](https://developer.apple.com/library/archive/samplecode/SerialPortSample/).
- Attribute registration and access in the [Max SDK](https://sdk.cdn.cycling74.com/max-sdk-8.2.0/group__attr.html).
- Editing object attributes with [attrui](https://docs.cycling74.com/reference/attrui/).
- Local reference implementation at `/Users/2bit/prog/maxproj/bbb.artnet/source/projects/bbb.artnet.controller/bbb.artnet.controller.cpp`.
