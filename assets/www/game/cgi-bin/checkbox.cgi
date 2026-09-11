#!/usr/bin/env python3
from _form_helpers import page, read_form

form = read_form()
page("Checkbox", ["Checkbox Maths is: " + ("ON" if form.get("maths") else "OFF"),
                  "Checkbox Physics is: " + ("ON" if form.get("physics") else "OFF")])
