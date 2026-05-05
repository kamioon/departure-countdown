# GNU ar (Homebrew binutils) creates archives with a GNU symbol-table '/' member
# that Apple's ld rejects.  Apple's ar rcs only produces '__.SYMDEF SORTED'.
# Work around it by extracting all .o members, deleting the archive, and
# recreating it with Apple's /usr/bin/ar just before the link step.
Import("env")

if env["PIOENV"] == "native":
    import glob
    import os
    import subprocess
    import tempfile
    import shutil

    APPLE_AR = "/usr/bin/ar"
    APPLE_RANLIB = "/usr/bin/ranlib"

    def _needs_fix(archive_path):
        result = subprocess.run(
            [APPLE_AR, "-t", archive_path],
            capture_output=True, text=True
        )
        return "/" in result.stdout.splitlines()

    def _rebuild_with_apple_ar(archive_path):
        members = [m for m in subprocess.run(
            [APPLE_AR, "-t", archive_path],
            capture_output=True, text=True
        ).stdout.splitlines() if m and m not in ("/", "__.SYMDEF", "__.SYMDEF SORTED")]

        if not members:
            return

        tmpdir = tempfile.mkdtemp()
        try:
            # Extract named .o members into a temp dir
            subprocess.run([APPLE_AR, "-x", archive_path] + members,
                           cwd=tmpdir, check=True)
            objects = [os.path.join(tmpdir, m) for m in members
                       if os.path.exists(os.path.join(tmpdir, m))]
            if not objects:
                return
            # Delete the GNU archive and recreate fresh with Apple ar
            os.remove(archive_path)
            subprocess.run([APPLE_AR, "rcs", archive_path] + objects, check=True)
            subprocess.run([APPLE_RANLIB, archive_path], check=True)
        finally:
            shutil.rmtree(tmpdir, ignore_errors=True)

    def fix_archives_before_link(source, target, env):
        build_dir = env.subst("$BUILD_DIR")
        for archive in glob.glob(os.path.join(build_dir, "**", "*.a"), recursive=True):
            if _needs_fix(archive):
                _rebuild_with_apple_ar(archive)

    env.AddPreAction("$BUILD_DIR/$PROGNAME$PROGSUFFIX", fix_archives_before_link)
