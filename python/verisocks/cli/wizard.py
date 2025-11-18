# MIT License
#
# Copyright (c) 2022-2025 Jérémie Chabloz
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

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
    parser.add_argument('--makefile-only', action='store_true',
                        help="Render makefile only (unless any other *-only \
option is being used)")
    parser.add_argument('--tb-only', action='store_true',
                        help="Render tesbench file only (unless any other \
*-only option is being used)")
    parser.add_argument('--vlt-only', action='store_true',
                        help="Render variables file only (unless any other \
*-only option is being used)")
    args = parser.parse_args()

    render_makefile = args.makefile_only or (
        not args.tb_only and not args.vlt_only)
    render_tb = args.tb_only or (
        not args.makefile_only and not args.vlt_only)
    render_vlt = args.vlt_only or (
        not args.makefile_only and not args.tb_only)

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
    if 'variables' in cfg:
        cfg['config']['verilog_src_files'] = (
            [str(args.variables_file)] + cfg['config']['verilog_src_files'])

    if render_makefile:
        if 'variables' in cfg:
            vlt_file = str(args.variables_file)
        else:
            vlt_file = None
        render_template(template_mk, args.makefile,
                        target_file=str(args.makefile),
                        config_file=str(args.config),
                        tb_file=str(args.testbench_file),
                        vlt_file=vlt_file,
                        **cfg['config'])
    if render_tb:
        if 'variables' in cfg:
            render_template(template_cpp, args.testbench_file,
                            **cfg['config'], variables=cfg['variables'])
        else:
            render_template(template_cpp, args.testbench_file,
                            **cfg['config'], variables=None)
    if render_vlt and ('variables' in cfg):
        render_template(template_vlt, args.variables_file,
                        variables=cfg['variables'])


if __name__ == "__main__":
    main()
