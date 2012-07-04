{
	"patcher" : 	{
		"fileversion" : 1,
		"appversion" : 		{
			"major" : 5,
			"minor" : 1,
			"revision" : 9
		}
,
		"rect" : [ 282.0, 229.0, 669.0, 757.0 ],
		"bglocked" : 0,
		"defrect" : [ 282.0, 229.0, 669.0, 757.0 ],
		"openrect" : [ 0.0, 0.0, 0.0, 0.0 ],
		"openinpresentation" : 0,
		"default_fontsize" : 12.0,
		"default_fontface" : 0,
		"default_fontname" : "Arial",
		"gridonopen" : 0,
		"gridsize" : [ 15.0, 15.0 ],
		"gridsnaponopen" : 0,
		"toolbarvisible" : 1,
		"boxanimatetime" : 200,
		"imprint" : 0,
		"enablehscroll" : 1,
		"enablevscroll" : 1,
		"devicewidth" : 0.0,
		"boxes" : [ 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "size 1000",
					"fontsize" : 12.0,
					"numinlets" : 2,
					"patching_rect" : [ 477.0, 1089.0, 62.0, 18.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"fontname" : "Arial",
					"id" : "obj-85"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "add buffer3 buffer1 buffer2",
					"fontsize" : 12.0,
					"numinlets" : 2,
					"patching_rect" : [ 25.272558, 1114.80603, 152.0, 18.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"fontname" : "Arial",
					"id" : "obj-81",
					"presentation_rect" : [ 25.272558, 1114.80603, 0.0, 0.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "reload.",
					"fontsize" : 12.0,
					"numinlets" : 2,
					"patching_rect" : [ 7.0, 1065.0, 47.0, 18.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"fontname" : "Arial",
					"id" : "obj-74"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "loadbang",
					"fontsize" : 12.0,
					"numinlets" : 1,
					"patching_rect" : [ 266.272583, 1045.0, 60.0, 20.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "bang" ],
					"fontname" : "Arial",
					"id" : "obj-9"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "toggle",
					"numinlets" : 1,
					"patching_rect" : [ 214.5, 1235.0, 20.0, 20.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "int" ],
					"id" : "obj-82"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "gate~",
					"fontsize" : 12.0,
					"numinlets" : 2,
					"patching_rect" : [ 214.5, 1264.583496, 41.0, 20.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "signal" ],
					"fontname" : "Arial",
					"id" : "obj-77"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "cycle~ 440",
					"fontsize" : 12.0,
					"numinlets" : 2,
					"patching_rect" : [ 236.5, 1235.0, 68.0, 20.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "signal" ],
					"fontname" : "Arial",
					"id" : "obj-76"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "ezdac~",
					"numinlets" : 2,
					"patching_rect" : [ 208.772568, 1296.583496, 45.0, 45.0 ],
					"numoutlets" : 0,
					"id" : "obj-75"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "gain 0.5",
					"fontsize" : 12.0,
					"numinlets" : 2,
					"patching_rect" : [ 100.77256, 1262.583496, 53.0, 18.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"fontname" : "Arial",
					"id" : "obj-37"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "pyext 1 0 sig gain",
					"fontsize" : 12.0,
					"numinlets" : 2,
					"patching_rect" : [ 38.77256, 1296.583496, 104.0, 20.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"fontname" : "Arial",
					"id" : "obj-39"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Signal Example",
					"fontsize" : 14.0,
					"numinlets" : 1,
					"patching_rect" : [ 15.272558, 1230.583496, 174.0, 23.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"id" : "obj-71"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "number",
					"fontsize" : 12.0,
					"numinlets" : 1,
					"patching_rect" : [ 480.0, 944.0, 50.0, 20.0 ],
					"numoutlets" : 2,
					"outlettype" : [ "int", "bang" ],
					"fontname" : "Arial",
					"id" : "obj-79"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "s TestSend2",
					"fontsize" : 12.0,
					"numinlets" : 1,
					"patching_rect" : [ 479.0, 975.0, 76.0, 20.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"id" : "obj-80"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "number",
					"fontsize" : 12.0,
					"numinlets" : 1,
					"patching_rect" : [ 181.493576, 890.0, 50.0, 20.0 ],
					"numoutlets" : 2,
					"outlettype" : [ "int", "bang" ],
					"fontname" : "Arial",
					"id" : "obj-78"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "number",
					"fontsize" : 12.0,
					"numinlets" : 1,
					"patching_rect" : [ 385.0, 944.0, 50.0, 20.0 ],
					"numoutlets" : 2,
					"outlettype" : [ "int", "bang" ],
					"fontname" : "Arial",
					"id" : "obj-72"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "s TestSend1",
					"fontsize" : 12.0,
					"numinlets" : 1,
					"patching_rect" : [ 384.0, 975.0, 76.0, 20.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"id" : "obj-70"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "print TestSend2",
					"fontsize" : 12.0,
					"numinlets" : 1,
					"patching_rect" : [ 480.0, 903.0, 94.0, 20.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"id" : "obj-69"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "print TestSend1",
					"fontsize" : 12.0,
					"numinlets" : 1,
					"patching_rect" : [ 385.0, 903.0, 94.0, 20.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"id" : "obj-68"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "r TestSend2",
					"fontsize" : 12.0,
					"numinlets" : 0,
					"patching_rect" : [ 480.0, 860.303955, 74.0, 20.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"fontname" : "Arial",
					"id" : "obj-63"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "r TestSend1",
					"fontsize" : 12.0,
					"numinlets" : 0,
					"patching_rect" : [ 385.0, 860.303955, 74.0, 20.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"fontname" : "Arial",
					"id" : "obj-62"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "print Sendrecv",
					"fontsize" : 12.0,
					"numinlets" : 1,
					"patching_rect" : [ 42.493576, 960.534729, 88.0, 20.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"id" : "obj-49"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "py sendrecv ex2 TestSend1",
					"fontsize" : 12.0,
					"numinlets" : 2,
					"patching_rect" : [ 42.493576, 932.303955, 158.0, 20.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"fontname" : "Arial",
					"id" : "obj-55"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Sendrecv Example",
					"fontsize" : 14.0,
					"numinlets" : 1,
					"patching_rect" : [ 21.0, 844.303955, 174.0, 23.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"id" : "obj-60"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "ret1",
					"fontsize" : 12.0,
					"numinlets" : 2,
					"patching_rect" : [ 30.0, 673.769226, 32.5, 18.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"fontname" : "Arial",
					"id" : "obj-33"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "strcat abc def",
					"fontsize" : 12.0,
					"numinlets" : 2,
					"patching_rect" : [ 122.75, 727.769226, 83.0, 18.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"fontname" : "Arial",
					"id" : "obj-7"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "strlen supercalafragialisticexpialadocious",
					"fontsize" : 12.0,
					"numinlets" : 2,
					"patching_rect" : [ 115.993576, 697.769226, 228.0, 18.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"fontname" : "Arial",
					"id" : "obj-6"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "print script",
					"fontsize" : 12.0,
					"numinlets" : 1,
					"patching_rect" : [ 42.493576, 756.0, 66.0, 20.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"id" : "obj-5"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "numargs a b c",
					"fontsize" : 12.0,
					"numinlets" : 2,
					"patching_rect" : [ 88.493576, 673.769226, 87.0, 18.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"fontname" : "Arial",
					"id" : "obj-65"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "py script",
					"fontsize" : 12.0,
					"numinlets" : 2,
					"patching_rect" : [ 42.493576, 727.769226, 55.0, 20.0 ],
					"numoutlets" : 2,
					"outlettype" : [ "", "" ],
					"fontname" : "Arial",
					"id" : "obj-66"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Script Example",
					"fontsize" : 14.0,
					"numinlets" : 1,
					"patching_rect" : [ 21.0, 639.769226, 174.0, 23.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"id" : "obj-67"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "mul buffer3 buffer1 buffer2",
					"fontsize" : 12.0,
					"numinlets" : 2,
					"patching_rect" : [ 26.272558, 1090.80603, 151.0, 18.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"fontname" : "Arial",
					"id" : "obj-64"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "waveform~",
					"setunit" : 1,
					"numinlets" : 5,
					"selectioncolor" : [ 0.313726, 0.498039, 0.807843, 0.0 ],
					"patching_rect" : [ 475.272583, 1141.80603, 134.0, 48.594055 ],
					"numoutlets" : 6,
					"bgcolor" : [ 0.501961, 0.717647, 0.764706, 1.0 ],
					"outlettype" : [ "float", "float", "float", "float", "list", "" ],
					"buffername" : "buffer3",
					"tickmarkcolor" : [ 0.352941, 0.337255, 0.521569, 1.0 ],
					"id" : "obj-59",
					"textcolor" : [  ],
					"waveformcolor" : [ 0.082353, 0.25098, 0.431373, 1.0 ],
					"labelbgcolor" : [ 0.352941, 0.337255, 0.521569, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "buffer~ buffer3",
					"fontsize" : 12.0,
					"numinlets" : 1,
					"patching_rect" : [ 475.272583, 1112.103027, 88.0, 20.0 ],
					"numoutlets" : 2,
					"outlettype" : [ "float", "bang" ],
					"fontname" : "Arial",
					"id" : "obj-61"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "waveform~",
					"setunit" : 1,
					"numinlets" : 5,
					"selectioncolor" : [ 0.313726, 0.498039, 0.807843, 0.0 ],
					"patching_rect" : [ 332.272583, 1141.80603, 134.0, 48.594055 ],
					"numoutlets" : 6,
					"bgcolor" : [ 0.501961, 0.717647, 0.764706, 1.0 ],
					"outlettype" : [ "float", "float", "float", "float", "list", "" ],
					"buffername" : "buffer2",
					"tickmarkcolor" : [ 0.352941, 0.337255, 0.521569, 1.0 ],
					"id" : "obj-58",
					"textcolor" : [  ],
					"waveformcolor" : [ 0.082353, 0.25098, 0.431373, 1.0 ],
					"labelbgcolor" : [ 0.352941, 0.337255, 0.521569, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "read rainstick.aif",
					"fontsize" : 13.0,
					"numinlets" : 2,
					"patching_rect" : [ 332.272583, 1087.80603, 105.0, 19.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"fontname" : "Arial",
					"id" : "obj-56"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "buffer~ buffer2",
					"fontsize" : 12.0,
					"numinlets" : 1,
					"patching_rect" : [ 332.272583, 1112.103027, 88.0, 20.0 ],
					"numoutlets" : 2,
					"outlettype" : [ "float", "bang" ],
					"fontname" : "Arial",
					"id" : "obj-57"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "read drumLoop.aif",
					"fontsize" : 13.0,
					"numinlets" : 2,
					"patching_rect" : [ 187.272568, 1087.80603, 119.0, 19.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"fontname" : "Arial",
					"id" : "obj-8"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "waveform~",
					"setunit" : 1,
					"numinlets" : 5,
					"selectioncolor" : [ 0.313726, 0.498039, 0.807843, 0.0 ],
					"patching_rect" : [ 187.272568, 1141.80603, 134.0, 48.594055 ],
					"numoutlets" : 6,
					"bgcolor" : [ 0.501961, 0.717647, 0.764706, 1.0 ],
					"outlettype" : [ "float", "float", "float", "float", "list", "" ],
					"buffername" : "buffer1",
					"tickmarkcolor" : [ 0.352941, 0.337255, 0.521569, 1.0 ],
					"id" : "obj-73",
					"textcolor" : [  ],
					"waveformcolor" : [ 0.082353, 0.25098, 0.431373, 1.0 ],
					"labelbgcolor" : [ 0.352941, 0.337255, 0.521569, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "buffer~ buffer1",
					"fontsize" : 12.0,
					"numinlets" : 1,
					"patching_rect" : [ 187.272568, 1112.103027, 88.0, 20.0 ],
					"numoutlets" : 2,
					"outlettype" : [ "float", "bang" ],
					"fontname" : "Arial",
					"id" : "obj-53"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "py buffer",
					"fontsize" : 12.0,
					"numinlets" : 2,
					"patching_rect" : [ 37.272556, 1159.80603, 57.0, 20.0 ],
					"numoutlets" : 2,
					"outlettype" : [ "", "" ],
					"fontname" : "Arial",
					"id" : "obj-52"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Buffer Example",
					"fontsize" : 14.0,
					"numinlets" : 1,
					"patching_rect" : [ 15.772556, 1031.80603, 174.0, 23.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"id" : "obj-51"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "threeargs foo bar maz",
					"fontsize" : 12.0,
					"numinlets" : 2,
					"patching_rect" : [ 250.5, 477.0, 128.0, 18.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"fontname" : "Arial",
					"id" : "obj-47"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "onearg start",
					"fontsize" : 12.0,
					"numinlets" : 2,
					"patching_rect" : [ 250.5, 428.0, 75.0, 18.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"fontname" : "Arial",
					"id" : "obj-45"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "twoargs 1 2",
					"fontsize" : 12.0,
					"numinlets" : 2,
					"patching_rect" : [ 250.5, 452.0, 73.0, 18.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"fontname" : "Arial",
					"id" : "obj-43"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "banana",
					"fontsize" : 12.0,
					"numinlets" : 2,
					"patching_rect" : [ 177.25, 477.0, 51.0, 18.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"fontname" : "Arial",
					"id" : "obj-36"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "ho",
					"fontsize" : 12.0,
					"numinlets" : 2,
					"patching_rect" : [ 177.25, 452.0, 32.5, 18.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"fontname" : "Arial",
					"id" : "obj-35"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "hey",
					"fontsize" : 12.0,
					"numinlets" : 2,
					"patching_rect" : [ 177.25, 428.0, 32.5, 18.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"fontname" : "Arial",
					"id" : "obj-34"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "1 2 3 foo",
					"fontsize" : 12.0,
					"numinlets" : 2,
					"patching_rect" : [ 77.5, 491.0, 57.0, 18.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"fontname" : "Arial",
					"id" : "obj-50"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "flonum",
					"fontsize" : 12.0,
					"numinlets" : 1,
					"patching_rect" : [ 49.0, 428.0, 50.0, 20.0 ],
					"numoutlets" : 2,
					"outlettype" : [ "float", "bang" ],
					"fontname" : "Arial",
					"id" : "obj-48"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "number",
					"fontsize" : 12.0,
					"numinlets" : 1,
					"patching_rect" : [ 68.5, 461.0, 50.0, 20.0 ],
					"numoutlets" : 2,
					"outlettype" : [ "int", "bang" ],
					"fontname" : "Arial",
					"id" : "obj-46"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "button",
					"numinlets" : 1,
					"patching_rect" : [ 21.0, 461.0, 20.0, 20.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "bang" ],
					"id" : "obj-44"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Simple Example",
					"fontsize" : 14.0,
					"numinlets" : 1,
					"patching_rect" : [ 21.0, 398.0, 174.0, 23.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"id" : "obj-42"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "print simple",
					"fontsize" : 12.0,
					"numinlets" : 1,
					"patching_rect" : [ 30.0, 560.0, 72.0, 20.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"id" : "obj-41"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Threading Example",
					"fontsize" : 14.0,
					"numinlets" : 1,
					"patching_rect" : [ 9.5, 75.0, 173.0, 23.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"id" : "obj-40"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "pyext simple ex1",
					"fontsize" : 12.0,
					"numinlets" : 4,
					"patching_rect" : [ 30.0, 531.0, 100.0, 20.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"fontname" : "Arial",
					"id" : "obj-38"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "fully parallel",
					"fontsize" : 9.0,
					"numinlets" : 1,
					"patching_rect" : [ 254.0, 120.0, 80.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"id" : "obj-1"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "with threads",
					"fontsize" : 9.0,
					"numinlets" : 1,
					"patching_rect" : [ 254.0, 104.0, 80.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"id" : "obj-2"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "t b 2",
					"fontsize" : 9.0,
					"numinlets" : 1,
					"patching_rect" : [ 267.0, 156.0, 27.0, 17.0 ],
					"numoutlets" : 2,
					"outlettype" : [ "bang", "int" ],
					"fontname" : "Arial",
					"id" : "obj-3"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "button",
					"numinlets" : 1,
					"patching_rect" : [ 266.0, 137.0, 15.0, 15.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "bang" ],
					"id" : "obj-4"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "py and pyext",
					"fontsize" : 24.0,
					"numinlets" : 1,
					"patching_rect" : [ 11.0, 9.0, 215.0, 34.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"id" : "obj-10"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Python externals ",
					"fontsize" : 9.0,
					"numinlets" : 1,
					"patching_rect" : [ 11.0, 42.0, 252.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"id" : "obj-11"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "you can even stop it...",
					"linecount" : 2,
					"fontsize" : 9.0,
					"numinlets" : 1,
					"patching_rect" : [ 350.0, 105.0, 80.0, 27.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"id" : "obj-12"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "pyext threads ex1 @detach 1",
					"fontsize" : 10.0,
					"numinlets" : 3,
					"patching_rect" : [ 285.0, 241.0, 141.0, 18.0 ],
					"numoutlets" : 3,
					"outlettype" : [ "", "", "" ],
					"fontname" : "Arial",
					"color" : [ 1.0, 0.360784, 0.682353, 1.0 ],
					"id" : "obj-13"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "non-blocking",
					"fontsize" : 9.0,
					"numinlets" : 1,
					"patching_rect" : [ 157.0, 119.0, 80.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"id" : "obj-14"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "with threads",
					"fontsize" : 9.0,
					"numinlets" : 1,
					"patching_rect" : [ 157.0, 103.0, 80.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"id" : "obj-15"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "--- blocking! ---",
					"fontsize" : 9.0,
					"numinlets" : 1,
					"patching_rect" : [ 45.0, 119.0, 80.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"id" : "obj-16"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "t b 1",
					"fontsize" : 9.0,
					"numinlets" : 1,
					"patching_rect" : [ 170.0, 155.0, 27.0, 17.0 ],
					"numoutlets" : 2,
					"outlettype" : [ "bang", "int" ],
					"fontname" : "Arial",
					"id" : "obj-17"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "t b 0",
					"fontsize" : 9.0,
					"numinlets" : 1,
					"patching_rect" : [ 70.0, 155.0, 27.0, 17.0 ],
					"numoutlets" : 2,
					"outlettype" : [ "bang", "int" ],
					"fontname" : "Arial",
					"id" : "obj-18"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "button",
					"numinlets" : 1,
					"patching_rect" : [ 169.0, 136.0, 15.0, 15.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "bang" ],
					"id" : "obj-19"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "button",
					"numinlets" : 1,
					"patching_rect" : [ 70.0, 136.0, 15.0, 15.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "bang" ],
					"id" : "obj-20"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "stop",
					"fontsize" : 12.0,
					"numinlets" : 2,
					"patching_rect" : [ 353.0, 136.0, 31.0, 18.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"fontname" : "Arial",
					"id" : "obj-21"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "doc+",
					"fontsize" : 12.0,
					"numinlets" : 2,
					"patching_rect" : [ 21.0, 209.53334, 35.0, 18.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"fontname" : "Arial",
					"id" : "obj-22"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "doc",
					"fontsize" : 12.0,
					"numinlets" : 2,
					"patching_rect" : [ 21.0, 185.266663, 28.0, 18.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"fontname" : "Arial",
					"id" : "obj-23"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "help",
					"fontsize" : 12.0,
					"numinlets" : 2,
					"patching_rect" : [ 21.0, 161.0, 39.0, 18.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"fontname" : "Arial",
					"id" : "obj-24"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "number",
					"fontsize" : 14.0,
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 151.0, 292.0, 35.0, 23.0 ],
					"numoutlets" : 2,
					"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
					"outlettype" : [ "int", "bang" ],
					"fontname" : "Arial",
					"triscale" : 0.9,
					"id" : "obj-25"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "number",
					"fontsize" : 14.0,
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 81.0, 293.0, 35.0, 23.0 ],
					"numoutlets" : 2,
					"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
					"outlettype" : [ "int", "bang" ],
					"fontname" : "Arial",
					"triscale" : 0.9,
					"id" : "obj-26"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "detach $1",
					"fontsize" : 12.0,
					"numinlets" : 2,
					"patching_rect" : [ 72.0, 210.0, 75.0, 18.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"fontname" : "Arial",
					"id" : "obj-27"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "pyext threads ex1",
					"fontsize" : 14.0,
					"numinlets" : 3,
					"patching_rect" : [ 79.0, 237.0, 121.0, 23.0 ],
					"numoutlets" : 3,
					"outlettype" : [ "", "", "" ],
					"fontname" : "Arial",
					"color" : [ 1.0, 0.360784, 0.682353, 1.0 ],
					"id" : "obj-28"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "watch that!",
					"fontsize" : 14.0,
					"numinlets" : 1,
					"patching_rect" : [ 91.0, 317.0, 100.0, 23.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"id" : "obj-29"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "without threads",
					"fontsize" : 9.0,
					"numinlets" : 1,
					"patching_rect" : [ 45.0, 103.0, 80.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"id" : "obj-30"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "panel",
					"numinlets" : 1,
					"patching_rect" : [ 5.0, 7.0, 626.0, 61.0 ],
					"grad1" : [ 0.633501, 0.929412, 0.919324, 1.0 ],
					"numoutlets" : 0,
					"bgcolor" : [ 0.952941, 0.564706, 0.282353, 1.0 ],
					"mode" : 1,
					"grad2" : [ 0.219255, 0.350905, 0.347096, 1.0 ],
					"id" : "obj-31"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "the detach state can be given as an attribute",
					"linecount" : 2,
					"fontsize" : 9.0,
					"numinlets" : 1,
					"patching_rect" : [ 286.0, 210.0, 110.0, 27.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"id" : "obj-32"
				}

			}
 ],
		"lines" : [ 			{
				"patchline" : 				{
					"source" : [ "obj-37", 0 ],
					"destination" : [ "obj-39", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-9", 0 ],
					"destination" : [ "obj-85", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-85", 0 ],
					"destination" : [ "obj-61", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-81", 0 ],
					"destination" : [ "obj-52", 1 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-74", 0 ],
					"destination" : [ "obj-52", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-64", 0 ],
					"destination" : [ "obj-52", 1 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-34", 0 ],
					"destination" : [ "obj-38", 2 ],
					"hidden" : 0,
					"midpoints" : [ 186.75, 447.0, 146.5, 447.0, 146.5, 528.0, 93.5, 528.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-35", 0 ],
					"destination" : [ "obj-38", 2 ],
					"hidden" : 0,
					"midpoints" : [ 186.75, 471.0, 146.5, 471.0, 146.5, 528.0, 93.5, 528.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-36", 0 ],
					"destination" : [ "obj-38", 2 ],
					"hidden" : 0,
					"midpoints" : [ 186.75, 528.0, 93.5, 528.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-38", 0 ],
					"destination" : [ "obj-41", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-43", 0 ],
					"destination" : [ "obj-38", 3 ],
					"hidden" : 0,
					"midpoints" : [ 260.0, 519.0, 120.5, 519.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-44", 0 ],
					"destination" : [ "obj-38", 1 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-45", 0 ],
					"destination" : [ "obj-38", 3 ],
					"hidden" : 0,
					"midpoints" : [ 260.0, 521.0, 120.5, 521.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-46", 0 ],
					"destination" : [ "obj-38", 1 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-47", 0 ],
					"destination" : [ "obj-38", 3 ],
					"hidden" : 0,
					"midpoints" : [ 260.0, 521.5, 120.5, 521.5 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-48", 0 ],
					"destination" : [ "obj-38", 1 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-50", 0 ],
					"destination" : [ "obj-38", 1 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-3", 0 ],
					"destination" : [ "obj-28", 1 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-3", 0 ],
					"destination" : [ "obj-28", 2 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-28", 0 ],
					"destination" : [ "obj-26", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-28", 1 ],
					"destination" : [ "obj-25", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-27", 0 ],
					"destination" : [ "obj-28", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-24", 0 ],
					"destination" : [ "obj-28", 0 ],
					"hidden" : 1,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-23", 0 ],
					"destination" : [ "obj-28", 0 ],
					"hidden" : 1,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-22", 0 ],
					"destination" : [ "obj-28", 0 ],
					"hidden" : 1,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-21", 0 ],
					"destination" : [ "obj-28", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-18", 0 ],
					"destination" : [ "obj-28", 1 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-18", 0 ],
					"destination" : [ "obj-28", 2 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-17", 0 ],
					"destination" : [ "obj-28", 1 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-17", 0 ],
					"destination" : [ "obj-28", 2 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-33", 0 ],
					"destination" : [ "obj-66", 1 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-6", 0 ],
					"destination" : [ "obj-66", 1 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-65", 0 ],
					"destination" : [ "obj-66", 1 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-66", 0 ],
					"destination" : [ "obj-5", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-7", 0 ],
					"destination" : [ "obj-66", 1 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-9", 0 ],
					"destination" : [ "obj-8", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-9", 0 ],
					"destination" : [ "obj-56", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-82", 0 ],
					"destination" : [ "obj-77", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-8", 0 ],
					"destination" : [ "obj-53", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-79", 0 ],
					"destination" : [ "obj-80", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-78", 0 ],
					"destination" : [ "obj-55", 1 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-77", 0 ],
					"destination" : [ "obj-75", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-77", 0 ],
					"destination" : [ "obj-75", 1 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-76", 0 ],
					"destination" : [ "obj-77", 1 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-72", 0 ],
					"destination" : [ "obj-70", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-63", 0 ],
					"destination" : [ "obj-69", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-62", 0 ],
					"destination" : [ "obj-68", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-56", 0 ],
					"destination" : [ "obj-57", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-55", 0 ],
					"destination" : [ "obj-49", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-4", 0 ],
					"destination" : [ "obj-3", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-3", 1 ],
					"destination" : [ "obj-27", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-20", 0 ],
					"destination" : [ "obj-18", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-19", 0 ],
					"destination" : [ "obj-17", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-18", 1 ],
					"destination" : [ "obj-27", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-17", 1 ],
					"destination" : [ "obj-27", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
 ]
	}

}
