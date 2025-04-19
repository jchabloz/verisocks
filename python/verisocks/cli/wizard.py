import argparse
import pathlib
import yaml
from os.path import isabs, abspath, dirname, join, relpath
from mako.template import Template


cwd = abspath(dirname(__file__))
tmpl_path = join(cwd, "templates")


def render_template(template_file, output_file, **kwargs):
    """Render Mako template

    Args:
        template_file (str): Path to template file
        output_file (str): Path to rendered output file
        kwargs: Keyword arguments required by the template
    """
    tmpl = Template(filename=template_file)
    tmpl_rendered = tmpl.render_unicode(
        template_filename=relpath(template_file, cwd), **kwargs)
    with open(output_file, 'w', encoding='utf-8') as f:
        f.write(tmpl_rendered)


def main():
    # Define arguments
    parser = argparse.ArgumentParser(description="""
This script generates automatically a Makefile, a C++ top testbench source file
and a Verilator configuration file to declare public variables for using
Verilator while integrating Verisocks. The files are generated based on Mako
templates and a YAML configuration file.
""")
    parser.add_argument('config', type=pathlib.Path,
                        help="YAML configuration file")
    parser.add_argument('--templates-dir', '-t', type=pathlib.Path,
                        default=tmpl_path,
                        help="Templates directory if alternative \
templates shall be used")
    parser.add_argument('--makefile', type=pathlib.Path, default="Makefile",
                        help="Rendered makefile name (default: Makefile)")
    parser.add_argument('--testbench-file', type=pathlib.Path,
                        default="test_main.cpp",
                        help="Rendered C++ testbench file (default: \
test_main.cpp)")
    parser.add_argument('--variables-file', type=pathlib.Path,
                        default="variables.vlt",
                        help="Rendered Verilator configuration file for \
public variables (default:variables.vlt)")
    args = parser.parse_args()

    # Format paths to templates relative to this file
    template_mk = join(args.templates_dir, "Makefile.mako")
    template_cpp = join(args.templates_dir, "test_main.cpp.mako")
    template_vlt = join(args.templates_dir, "variables.vlt.mako")

    # Load JSON configuration file
    with open(args.config, 'r') as f:
        cfg = yaml.safe_load(f)

    # Format relative paths in config file
    def format_path(x):
        if isabs(x):
            return x
        return abspath(join(dirname(args.config), x))
    for k in ["verisocks_root", "verilator_root", "verilator_path"]:
        format_path(cfg['config'][k])

    # Add C++ top testbench file at the front of C++ sources list
    if 'cpp_src_files' not in cfg['config']:
        cfg['config']['cpp_src_files'] = []
    cfg['config']['cpp_src_files'] = (
        [str(args.testbench_file)] + cfg['config']['cpp_src_files'])

    # Add verilator configuration file for public variables at the front of the
    # Verilog sources list
    cfg['config']['verilog_src_files'] = (
        [str(args.variables_file)] + cfg['config']['verilog_src_files'])

    render_template(template_mk, args.makefile, **cfg['config'])
    render_template(template_cpp, args.testbench_file,
                    **cfg['config'], variables=cfg['variables'])
    render_template(template_vlt, args.variables_file,
                    variables=cfg['variables'])


if __name__ == "__main__":
    main()
