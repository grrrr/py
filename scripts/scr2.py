import Tkinter

print "Script scr2.py initialized"

def addall(*args):   # variable argument list
	s = 0
	for si in args:
		s += si
	return s


def ret1():
	return 1,2,3,4


def ret2():
	return "sd","lk","ki"


def ret3():
	return ["sd","lk","ki"]



def main():
    global root, history_frame, info_line
    root = Tkinter.Tk()
    root.title('Txt2Html TK Shell')
#    init_vars()

    #-- Create the menu frame, and menus to the menu frame
#    menu_frame = Tkinter.Frame(root)
#    menu_frame.pack(fill=Tkinter.X, side=Tkinter.TOP)
#    menu_frame.tk_menuBar(file_menu(), action_menu(), help_menu())

    #-- Create the history frame (to be filled in during runtime)
#    history_frame = Tkinter.Frame(root)
#    history_frame.pack(fill=Tkinter.X, side=Tkinter.BOTTOM, pady=2)

    #-- Create the info frame and fill with initial contents
    info_frame = Tkinter.Frame(root)
    info_frame.pack(fill=Tkinter.X, side=Tkinter.BOTTOM)

    # first put the column labels in a sub-frame
    LEFT, Label = Tkinter.LEFT, Tkinter.Label   # shortcut names
    label_line = Tkinter.Frame(info_frame, relief=Tkinter.RAISED, borderwidth=1)
    label_line.pack(side=Tkinter.TOP, padx=2, pady=1)
    Label(label_line, text="Run #", width=5).pack(side=LEFT)
    Label(label_line, text="Source:", width=20).pack(side=LEFT)
    Label(label_line, text="Target:", width=20).pack(side=LEFT)
    Label(label_line, text="Type:", width=20).pack(side=LEFT)
    Label(label_line, text="Proxy Mode:", width=20).pack(side=LEFT)

    # then put the "next run" information in a sub-frame
    info_line = Tkinter.Frame(info_frame)
    info_line.pack(side=Tkinter.TOP, padx=2, pady=1)
#    update_specs()

    #-- Finally, let's actually do all that stuff created above
    root.mainloop()


