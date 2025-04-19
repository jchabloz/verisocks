<%!
def get_var_name(path):
	return path.split(".")[-1]
%>\
//Note: This file has been generated from the template ${template_filename}
`verilator_config
% for vars in variables.values():
% for var in vars:
public -module "${var['module']}" -var "${get_var_name(var['path'])}"
% endfor
% endfor
