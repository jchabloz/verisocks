import subprocess
import os.path
import shutil
import socket
import logging


def find_free_port():
    """Find a free port on localhost

    The implementation for this function is not very elegant, but it does the
    job... It uses the property of :py:meth:`socket.socket.bind` method to bind
    the socket to a randomly-assigned free port when provided a :code:`('', 0)`
    address argument. Using :py:meth:`socket.socket.getsockname` is then used
    to retrieve the corresponding port number. Since the bound socket is closed
    within the function, it is assumed that the same port number should also be
    free again; this is where the weakness of this method lies, since race
    conditions cannot be fully excluded.
    """
    with socket.socket() as s:
        s.bind(('', 0))
        retval = s.getsockname()[1]
    return retval


def _format_path(cwd, path):
    if os.path.isabs(path):
        return path
    return os.path.abspath(os.path.join(cwd, path))


def setup_sim(vpi_libpath, *src_files, cwd=".", vvp_filepath=None,
              ivl_exec=None, ivl_args=[], vvp_exec=None, vvp_args=[],
              capture_output=True):
    """Set up Icarus simulation by elaborating the design with :code:`iverilog`
    and launching the simulation with :code:`vvp`.

    Args:
        cwd (str): Reference path to be used for all paths provided as relative
            paths. Default = "."
        vpi_libpath (str): Path to the compiled Verisocks VPI library (absolute
            path).
        src_files (str): Paths to all (verilog) source files to use for the
            simulation. All files have to be added as separate arguments.
        vvp_filepath (str): Path to the elaborated VVP file (iverilog output).
            If None (default), the sim.vvp will be used. The path is relative
            to the current directory.
        ivl_exec (str): Path to :code:`iverilog` executable (absolute path). If
            None (default), it is assumed to be defined in the system path.
        ivl_args (list(str)): Arguments to :code:`iverilog` executable.
            Default=[] (no extra arguments).
        vvp_exec (str): Path to :code:`vvp` executable (absolute path). If None
            (default), it is assumed to be defined in the system path.
        vvp_args (list(str)): Arguments to :code:`vvp` executable. Default=[].
        capture_output (bool): Defines if stdout and stderr output are
            "captured" (i.e. not visible).

    Returns:
        subprocess.Popen
    """

    vpi_libpath = _format_path(cwd, vpi_libpath)
    if not os.path.isfile(vpi_libpath):
        raise FileNotFoundError(f"Could not find {vpi_libpath}")

    src_file_paths = []
    for src_file in src_files:
        src_file_path = _format_path(cwd, src_file)
        if not os.path.isfile(src_file_path):
            raise FileNotFoundError(f"File {src_file_path} not found")
        src_file_paths.append(src_file_path)

    if vvp_filepath:
        vvp_outfile = _format_path(cwd, vvp_filepath)
    else:
        vvp_outfile = _format_path(cwd, "sim.vvp")

    # Elaboration with iverilog
    if ivl_exec:
        ivl_cmd = [_format_path(cwd, ivl_exec)]
    else:
        ivl_cmd = [shutil.which("iverilog")]

    ivl_cmd += [
        "-o", vvp_outfile,
        "-Wall",
        *ivl_args,
        *src_file_paths
    ]
    subprocess.check_call(ivl_cmd)

    # Simulation with vvp
    if vvp_exec:
        vvp_cmd = [_format_path(cwd, vvp_exec)]
    else:
        vvp_cmd = [shutil.which("vvp")]

    vvp_cmd += [
        "-lvvp.log",
        "-m", vpi_libpath,
        *vvp_args,
        vvp_outfile,
        "-fst"
    ]

    if capture_output:
        pop = subprocess.Popen(
            vvp_cmd,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL
        )
    else:
        pop = subprocess.Popen(vvp_cmd)

    logging.info(f"Launched Icarus with PID {pop.pid}")
    return pop
