#!/usr/bin/env python3
from _form_helpers import page, read_form

form = read_form()
page("Dropdown Box", ["Selected Subject is: " + (form.get("dropdown") or "Not entered")])
