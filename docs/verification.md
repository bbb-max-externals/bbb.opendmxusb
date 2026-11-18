# Verification — 2026-11-18

## Confirmed

- macOS arm64 + x86_64 Universal Binary builds successfully. Both slices declare macOS 11.0 minimum. Ad-hoc code signature verifies.
- Focused engine test passes: 513-byte frame/start-code boundaries, shortened frames, blackout preserves the stored levels, bang/change/forced behavior, framerate change wake-up, send/open failure, reconnect, close and worker shutdown.
- Max 9.1.5 loads the external. The initial integration test exposed an incorrect attribute-name accessor; it was fixed to use the SDK's `getname` method.
- With that correction, the real Max process confirms `@mode 1`, `@num_channels 4`, `origin 0`, `blackout 1`, clipped input, setchannel/set_offset, `dump`, serial enumeration, status and a missing-port error. See [max-smoke-result.json](max-smoke-result.json).
- `attrui @attr port` displays the selected path and can step between the enumerated ports. `ports` / `refresh` update per-instance `enumvals` metadata through `object_attr_attr_setvalueof`.
- The error is also available from macOS Unified Log with subsystem `jp.2bit.bbb.opendmxusb`.

## Test conditions and limits

The existing Max process retained its first loaded external. To preserve the user's other open patch, the corrected source was compiled under the test-only class name `bbb.opendmxusb.check`. This changes only the registered class name; it is not part of the package API. The final installed name remains `bbb.opendmxusb`. Restart Max after saving work to load the final binary.

The Max JSON result was normalized from the first complete JSON value in the test log: Max's legacy JS `File` does not truncate an existing longer file automatically. The test writer now explicitly sets `eof = 0`.

The last build additionally clears all macOS modem flow-control flags and sets the deployment target to 11.0; those transport settings were compiled but have not been exercised against an ENTTEC device. The signature and engine tests apply to the final build.

No physical DMX receiver capture, lighting fixture response, BREAK/MAB oscilloscope trace, or sustained hardware refresh measurement was performed. Seeing an FTDI serial port is not proof that it is an ENTTEC OPEN DMX USB. The current implementation uses the VCP driver; it does not contain D2XX, USB PRO support, RDM, or a Windows backend.

## Reproduce

Build with `-DOPEN_DMX_BUILD_TESTS=ON`, then run `ctest --test-dir build --output-on-failure`.

Install the package, restart Max, create `artifacts/`, and open `tests/max-smoke.maxpat`. Check that each `passed` field in `artifacts/max-smoke.json` is true and that a missing-port error appears. The patch leaves the selected port at `none`.

For hardware acceptance, select the known ENTTEC port, use forced mode at 40 fps, and observe received levels/start code and line timing using a DMX receiver or analyzer. Check blackout/restore and unplug/reconnect. An output bang acknowledges completion of the serial send routine, not fixture reception.
