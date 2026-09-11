#!/usr/bin/env python3
from _form_helpers import page, read_form

form = read_form()
page("Radio Button", ["Selected Subject is: " + (form.get("subject") or "Not entered")])
