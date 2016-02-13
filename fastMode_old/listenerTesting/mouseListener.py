#!/usr/bin/python
from Xlib import display

while True:
  c = display.Display().screen().root.query_pointer()._data
  print c["root_x"], c["root_y"]
