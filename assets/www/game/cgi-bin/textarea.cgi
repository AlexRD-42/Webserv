#!/usr/bin/env python3
from _form_helpers import page, read_form

form = read_form()
page("Text Area", ["Entered Text Content is: " + (form.get("textcontent") or "Not entered")])
