#
#      NODE Navigator v0.94
#
#      Python version, which uses John Zelle's graphics.py
#      ported from Watcom C++32 compiler version
#
#      Frank Palazzolo
#      frank@avoidspikes.com
#

from sys import argv
from struct import Struct
from collections import namedtuple
from graphics import GraphWin, Point, Line, Rectangle, Text

# Find a WAD file directory entry
def seek_entry(fp, name):
    extname = name.encode('ascii') + b'\x00'*(8-len(name))

    DirentryFmt = Struct('=ii8s')
    DirentryType = namedtuple('Direntry','startaddr length name')

    while True:
        buf = fp.read(DirentryFmt.size)
        if buf == b'':
            return 0    # not found
        entry = DirentryType._make(DirentryFmt.unpack(buf))
        if entry.name == extname:
            return entry

# Read a WAD file resource, preserving the current file pointer
def read_multi(fp, entry, fmt, type):
    rv = []
    ptr = fp.tell()
    fp.seek(entry.startaddr)
    num_linedefs = entry.length // fmt.size
    for i in range(0,num_linedefs):
        buf = fp.read(fmt.size)
        rv.append(type._make(fmt.unpack(buf)))
    fp.seek(ptr)
    return rv

def main():
    # Handle args
    if len(argv) < 2:
       print(f'Usage: {argv[0]} mission [wadfile]');
       return -1
    elif len(argv) == 2:
        filename = 'DOOM.WAD'
    else:
        filename = argv[2].upper()
    mission = argv[1].upper()

    # Read WAD file
    try:
        print(f'Reading file {filename}...')
        with open(filename,"rb") as fp:
            
            # Read header
            HeaderFmt = Struct('=4sii')
            HeaderType = namedtuple('Header','type num_dir_entries dirpointer')
            buf = fp.read(HeaderFmt.size)
            wadheader = HeaderType._make(HeaderFmt.unpack(buf))
            print(f'File ID: {wadheader.type.decode("ascii")}')
            print(f'# of Directory entries: {wadheader.num_dir_entries}');

            # Find mission
            fp.seek(wadheader.dirpointer)
            entry = seek_entry(fp, mission)
            if entry == 0:
                print(f'Could not find {mission} in {filename}')
                return -1
            else:
                print(f'Found {mission}...')

            # Load resources

            LinedefFmt = Struct('=hhhhhhh')
            LinedefType = namedtuple('Linedef','from_vertex to_vertex attributes type sector_trigger right_sidedef left_sidedef')
            direntry = seek_entry(fp, "LINEDEFS")
            linedefs = read_multi(fp, direntry, LinedefFmt, LinedefType)
            print(f'Read in {len(linedefs)} linedefs...')

            SidedefFmt = Struct('=hh8s8s8sh')
            SidedefType = namedtuple('Sidedef','u_offset v_offset uppertxt lowertxt walltxt sector')
            direntry = seek_entry(fp, "SIDEDEFS")
            sidedefs = read_multi(fp, direntry, SidedefFmt, SidedefType)
            print(f'Read in {len(sidedefs)} sidedefs...')

            VertexFmt = Struct('=hh')
            VertexType = namedtuple('Vertex','x y')
            direntry = seek_entry(fp, "VERTEXES")
            vertices = read_multi(fp, direntry, VertexFmt, VertexType)
            print(f'Read in {len(vertices)} vertices...')

            SegmentFmt = Struct('=hhhhhh')
            SegmentType = namedtuple('Segment','from_vertex to_vertex angle linedef side distance')
            direntry = seek_entry(fp, "SEGS")
            segment = read_multi(fp, direntry, SegmentFmt, SegmentType)
            print(f'Read in {len(segment)} segments...')

            SSectorFmt = Struct('=hh')
            SSectorType = namedtuple('SSector','startseg numsegs')
            direntry = seek_entry(fp, "SSECTORS")
            ssectors = read_multi(fp, direntry, SSectorFmt, SSectorType)
            print(f'Read in {len(ssectors)} ssectors...')

            NodeFmt = Struct('=hhhhhhhhhhhhHH')
            NodeType = namedtuple('Node','x y dx dy left_y_upper left_y_lower left_x_upper left_x_lower right_y_upper right_y_lower right_x_upper right_x_lower left_child right_child')
            direntry = seek_entry(fp, "NODES")
            node = read_multi(fp, direntry, NodeFmt, NodeType)
            print(f'Read in {len(node)} nodes...')

            SectorFmt = Struct('=hh8s8shhh')
            SectorType = namedtuple('Sector','floor_alt ceiling_alt floortxt ceiltxt brightness special trigger')
            direntry = seek_entry(fp, "SECTORS")
            sectors = read_multi(fp, direntry, SectorFmt, SectorType)
            print(f'Read in {len(sectors)} sectors...')
    except:
        print(f'Error reading file {filename}')
        return -1
    
    # Initialize parent nodes
    parent_node = [0]*len(node)
    parent_node[len(node)-1] = len(node)-1

    for i in range(0,len(node)):
        if node[i].left_child & 0x8000 == 0x0000:
                parent_node[node[i].left_child] = i
        if node[i].right_child & 0x8000 == 0x0000:
                parent_node[node[i].right_child] = i
    
    # Window size
    width = 640
    height = 480

    # Create window
    win = GraphWin("NodeNav - 30th Anniversary Edition (1994-2024)", width, height, autoflush=False)
    win.setBackground("black")
    
    # Init loop vars
    idx = len(node) - 1     # Node Index
    n = node[idx]           # Current node
    key = ''                # Last key hit

    # Main loop
    while True:

        # Calculate scaling based on size of current node
        min_x = min(min(n.left_x_upper, n.left_x_lower),
                    min(n.right_x_upper,n.right_x_lower))

        min_y = min(min(n.left_y_upper, n.left_y_lower),
                    min(n.right_y_upper,n.right_y_lower))

        max_x = max(max(n.left_x_upper, n.left_x_lower),
                    max(n.right_x_upper,n.right_x_lower))

        max_y = max(max(n.left_y_upper, n.left_y_lower),
                    max(n.right_y_upper,n.right_y_lower))

        mid_x = (min_x + max_x) / 2
        mid_y = (min_y + max_y) / 2

        # Leave some margin
        scale_x = (width / 2. / (max_x - mid_x)) * 0.90
        scale_y = (height / 2. / (max_y - mid_y)) * 0.90

        # Fit to whatever dimension is longest
        if scale_x < scale_y:
           scale = scale_x
        else:
           scale = scale_y

        # Init child node types
        if n.left_child & 0x8000:
           L_type = 'S'
        else:
           L_type = 'N'
        if n.right_child & 0x8000:
           R_type = 'S'
        else:
           R_type = 'N'
        
        # Clear everything on screen
        for item in win.items[:]:
            item.undraw()

        # Reset coord transform to new zoomlevel
        win.setCoords(mid_x-(width/2)/scale, 
                      mid_y-(height/2)/scale, 
                      mid_x+(width/2)/scale, 
                      mid_y+(height/2)/scale)
        
        # Gray Map
        for i in range(0,len(linedefs)):
            line = Line(
                Point(vertices[linedefs[i].from_vertex].x,
                      vertices[linedefs[i].from_vertex].y),
                Point(vertices[linedefs[i].to_vertex].x,
                      vertices[linedefs[i].to_vertex].y)
                )
            line.setOutline("gray")
            line.draw(win)

        # Red Rect
        rect = Rectangle(
            Point(n.left_x_upper, n.left_y_upper),
            Point(n.left_x_lower, n.left_y_lower)
        )
        rect.setOutline("red")
        rect.draw(win)

        # Blue Rect
        rect = Rectangle(
            Point(n.right_x_upper, n.right_y_upper),
            Point(n.right_x_lower, n.right_y_lower)
        )
        rect.setOutline("blue")
        rect.draw(win)

        # Green Line
        line = Line(
            Point(n.x,n.y),
            Point(n.x+n.dx,n.y+n.dy)
        )
        line.setOutline("lightgreen")
        line.draw(win)
        
        # Text
        hstring = f'NodeNav v0.94           N:0x{idx:04x}     {L_type}:0x{n.left_child & 0x7fff:04x}   {R_type}:0x{n.right_child & 0x7fff:04x}'
        #heading = Text(Point(width/2, 10), hstring)
        heading = Text(Point(mid_x, (10-height/2)/(-scale)+mid_y), hstring) # have to undo the scaling here...awkward
        heading.setOutline("white")
        heading.draw(win)

        # Block here - Read a key or exception if window closed
        try:
            # Note - with autoflush=False, getKey() actually does the drawing, all at once
            key = win.getKey().upper()
        except: # In case you close the window
            break

        # Process keypress
        if key == 'Q':
            break
        elif key == 'L' and L_type == 'N':
            idx = n.left_child
        elif key == 'R' and R_type == 'N':
            idx = n.right_child
        elif key == 'U':
            idx = parent_node[idx]

        # update current node
        n = node[idx]

    # Exit
    win.close()
    return 0

if __name__ == "__main__":
    main()
