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
    "title": "DMX API Check",
    "description": "",
    "digest": "",
    "tags": "",
    "style": "",
    "subpatcher_template": "",
    "assistshowspatchername": 0,
    "boxes": [
      {
        "box": {
          "id": "obj-start",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            30,
            30,
            80,
            22
          ],
          "outlettype": [
            "bang"
          ],
          "text": "loadbang"
        }
      },
      {
        "box": {
          "id": "obj-run",
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
          "text": "run"
        }
      },
      {
        "box": {
          "id": "obj-smoke",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            400,
            150,
            80,
            22
          ],
          "outlettype": [
            ""
          ],
          "text": "js max-smoke.js"
        }
      }
    ],
    "lines": [
      {
        "patchline": {
          "source": [
            "obj-start",
            0
          ],
          "destination": [
            "obj-run",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-run",
            0
          ],
          "destination": [
            "obj-smoke",
            0
          ]
        }
      }
    ]
  }
}