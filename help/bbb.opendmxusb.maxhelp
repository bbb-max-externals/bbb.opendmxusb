{
  "patcher": {
    "fileversion": 1,
    "appversion": {
      "major": 9,
      "minor": 0,
      "revision": 0,
      "architecture": "x64",
      "modernui": 1
    },
    "classnamespace": "box",
    "rect": [
      100,
      100,
      640,
      480
    ],
    "bglocked": 0,
    "openrect": [
      0,
      0,
      0,
      0
    ],
    "openinpresentation": 0,
    "default_fontsize": 12,
    "default_fontface": 0,
    "default_fontname": "Arial",
    "gridonopen": 2,
    "gridsize": [
      15,
      15
    ],
    "gridsnaponopen": 0,
    "objectsnaponopen": 1,
    "statusbarvisible": 2,
    "toolbarvisible": 2,
    "lefttoolbarpinned": 0,
    "toptoolbarpinned": 0,
    "righttoolbarpinned": 0,
    "bottomtoolbarpinned": 0,
    "toolbars_unpinned_last_save": 0,
    "tallnewobj": 0,
    "boxanimatetime": 200,
    "enablehscroll": 1,
    "enablevscroll": 1,
    "devicewidth": 0,
    "title": "Serial DMX Output",
    "description": "",
    "digest": "",
    "tags": "",
    "style": "",
    "subpatcher_template": "",
    "assistshowspatchername": 0,
    "boxes": [
      {
        "box": {
          "id": "obj-intro",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            30,
            20,
            760,
            22
          ],
          "text": "Select a serial port, then send DMX levels. Disconnect with close or port none."
        }
      },
      {
        "box": {
          "id": "obj-ports",
          "maxclass": "message",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            30,
            65,
            80,
            22
          ],
          "outlettype": [
            ""
          ],
          "text": "ports"
        }
      },
      {
        "box": {
          "id": "obj-refresh",
          "maxclass": "message",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            95,
            65,
            80,
            22
          ],
          "outlettype": [
            ""
          ],
          "text": "refresh"
        }
      },
      {
        "box": {
          "id": "obj-port",
          "maxclass": "attrui",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            180,
            65,
            480,
            22
          ],
          "outlettype": [
            ""
          ],
          "attr": "port"
        }
      },
      {
        "box": {
          "id": "obj-open",
          "maxclass": "message",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            680,
            65,
            80,
            22
          ],
          "outlettype": [
            ""
          ],
          "text": "open"
        }
      },
      {
        "box": {
          "id": "obj-close",
          "maxclass": "message",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            740,
            65,
            80,
            22
          ],
          "outlettype": [
            ""
          ],
          "text": "close"
        }
      },
      {
        "box": {
          "id": "obj-mode",
          "maxclass": "attrui",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            30,
            120,
            240,
            22
          ],
          "outlettype": [
            ""
          ],
          "attr": "mode"
        }
      },
      {
        "box": {
          "id": "obj-rate",
          "maxclass": "attrui",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            290,
            120,
            200,
            22
          ],
          "outlettype": [
            ""
          ],
          "attr": "framerate"
        }
      },
      {
        "box": {
          "id": "obj-channels",
          "maxclass": "attrui",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            510,
            120,
            200,
            22
          ],
          "outlettype": [
            ""
          ],
          "attr": "num_channels"
        }
      },
      {
        "box": {
          "id": "obj-blackout",
          "maxclass": "attrui",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            30,
            165,
            240,
            22
          ],
          "outlettype": [
            ""
          ],
          "attr": "blackout"
        }
      },
      {
        "box": {
          "id": "obj-origin",
          "maxclass": "attrui",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            290,
            165,
            200,
            22
          ],
          "outlettype": [
            ""
          ],
          "attr": "origin"
        }
      },
      {
        "box": {
          "id": "obj-mode_note",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            30,
            205,
            760,
            22
          ],
          "text": "Default: forced (40 fps). Other modes send only on messages; fixtures may time out."
        }
      },
      {
        "box": {
          "id": "obj-levels",
          "maxclass": "message",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            30,
            250,
            80,
            22
          ],
          "outlettype": [
            ""
          ],
          "text": "255 128 0 0"
        }
      },
      {
        "box": {
          "id": "obj-channel",
          "maxclass": "message",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            180,
            250,
            80,
            22
          ],
          "outlettype": [
            ""
          ],
          "text": "channel 1 255"
        }
      },
      {
        "box": {
          "id": "obj-store",
          "maxclass": "message",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            330,
            250,
            80,
            22
          ],
          "outlettype": [
            ""
          ],
          "text": "set 0 128 255 0"
        }
      },
      {
        "box": {
          "id": "obj-store_channel",
          "maxclass": "message",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            500,
            250,
            80,
            22
          ],
          "outlettype": [
            ""
          ],
          "text": "setchannel 1 0"
        }
      },
      {
        "box": {
          "id": "obj-offset",
          "maxclass": "message",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            650,
            250,
            80,
            22
          ],
          "outlettype": [
            ""
          ],
          "text": "set_offset 2 255 128 0"
        }
      },
      {
        "box": {
          "id": "obj-bang",
          "maxclass": "button",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            30,
            300,
            24,
            24
          ],
          "outlettype": [
            "bang"
          ]
        }
      },
      {
        "box": {
          "id": "obj-dump",
          "maxclass": "message",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            95,
            300,
            80,
            22
          ],
          "outlettype": [
            ""
          ],
          "text": "dump"
        }
      },
      {
        "box": {
          "id": "obj-status",
          "maxclass": "message",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            180,
            300,
            80,
            22
          ],
          "outlettype": [
            ""
          ],
          "text": "status"
        }
      },
      {
        "box": {
          "id": "obj-store_note",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            30,
            345,
            760,
            22
          ],
          "text": "set / setchannel / set_offset store values. In forced mode the next refresh uses them."
        }
      },
      {
        "box": {
          "id": "obj-controller",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 2,
          "patching_rect": [
            30,
            410,
            180,
            22
          ],
          "outlettype": [
            "",
            ""
          ],
          "text": "bbb.opendmxusb"
        }
      },
      {
        "box": {
          "id": "obj-frame_or_dump",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            30,
            470,
            80,
            22
          ],
          "text": "print opendmx.data"
        }
      },
      {
        "box": {
          "id": "obj-port_status",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            350,
            470,
            80,
            22
          ],
          "text": "print opendmx.status"
        }
      },
      {
        "box": {
          "id": "obj-limits",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            30,
            525,
            780,
            22
          ],
          "text": "OPEN DMX USB (FTDI virtual serial port), macOS. USB PRO and RDM are not supported."
        }
      }
    ],
    "lines": [
      {
        "patchline": {
          "source": [
            "obj-ports",
            0
          ],
          "destination": [
            "obj-controller",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-refresh",
            0
          ],
          "destination": [
            "obj-controller",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-port",
            0
          ],
          "destination": [
            "obj-controller",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-open",
            0
          ],
          "destination": [
            "obj-controller",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-close",
            0
          ],
          "destination": [
            "obj-controller",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-mode",
            0
          ],
          "destination": [
            "obj-controller",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-rate",
            0
          ],
          "destination": [
            "obj-controller",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-channels",
            0
          ],
          "destination": [
            "obj-controller",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-blackout",
            0
          ],
          "destination": [
            "obj-controller",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-origin",
            0
          ],
          "destination": [
            "obj-controller",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-levels",
            0
          ],
          "destination": [
            "obj-controller",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-channel",
            0
          ],
          "destination": [
            "obj-controller",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-store",
            0
          ],
          "destination": [
            "obj-controller",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-store_channel",
            0
          ],
          "destination": [
            "obj-controller",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-offset",
            0
          ],
          "destination": [
            "obj-controller",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-bang",
            0
          ],
          "destination": [
            "obj-controller",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-dump",
            0
          ],
          "destination": [
            "obj-controller",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-status",
            0
          ],
          "destination": [
            "obj-controller",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-controller",
            0
          ],
          "destination": [
            "obj-frame_or_dump",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-controller",
            1
          ],
          "destination": [
            "obj-port_status",
            0
          ]
        }
      }
    ]
  }
}