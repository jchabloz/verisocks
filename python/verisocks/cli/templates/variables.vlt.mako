<%!
def get_var_name(path):
	return path.split(".")[-1]
%>\
`verilator_config
% for vars in variables.values():
% for var in vars:
public -module "${var['module']}" -var "${get_var_name(var['path'])}"
% endfor
% endfor
