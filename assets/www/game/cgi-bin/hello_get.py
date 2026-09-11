#!/usr/bin/env python3
from _form_helpers import page, read_form

form = read_form()
name = " ".join(filter(None, [form.get("first_name", ""), form.get("last_name", "")]))
page("Hello", ["Hello " + (name or "penguin")])
