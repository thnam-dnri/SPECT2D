#!/usr/bin/env python3
import os
import subprocess
import time

def submit_check(keyword):
    """Returns the number of lines from `qstat` containing the given keyword."""
    try:
        output = subprocess.check_output(
            f"qstat | grep {keyword}", shell=True,
            universal_newlines=True,
        )
        lines = [line for line in output.splitlines() if line.strip()]
        return len(lines)
    except subprocess.CalledProcessError:
        return 0

def main():
    # Configuration
    jobmax   = 70        # max jobs allowed in queue "BS"
    myjobmax = 50        # max jobs allowed for user
    waittime = 30        # seconds between queue checks

    # Scan parameters
    step_size = 1.68     # mm
    x_steps   = 40
    y_steps   = 20
    z_offset  = 0.0      # mm

    # Rotation angles (0 to 180 by 45)
    rotation_angles = list(range(0, 181, 45))

    # Directories
    base_dir   = os.path.abspath(os.path.dirname(__file__))
    mac_sh_dir = os.path.join(base_dir, "mac_sh")
    os.makedirs(mac_sh_dir, exist_ok=True)
    build_dir  = os.path.expanduser("~/Geant4/SPEC2D/build")

    # Position grids
    x_positions = [(-step_size * (x_steps - 1) / 2.0 + i * step_size) for i in range(x_steps)]
    y_positions = [(-step_size * (y_steps - 1) / 2.0 + j * step_size) for j in range(y_steps)]

    count = 0
    for x in x_positions:
        for y in y_positions:
            for rot in rotation_angles:
                # Format values
                x_str   = f"{x:.2f}"
                y_str   = f"{y:.2f}"
                z_str   = f"{z_offset:.1f}"
                rot_str = f"{rot:.0f}"

                # Create .mac file
                mac_filename = f"run_x_{x_str}_y_{y_str}_zoff_{z_str}_rot_{rot_str}.mac"
                mac_path     = os.path.join(mac_sh_dir, mac_filename)
                with open(mac_path, 'w') as mac_file:
                    mac_file.write(f"/simulation/offset  {x_str}  {y_str}  {z_str} mm\n")
                    mac_file.write(f"/simulation/rotate  {rot_str}  deg\n")
                    mac_file.write("/run/initialize\n")
                    mac_file.write("/run/beamOn        100000\n")

                # Create .sh file
                sh_filename = mac_filename.replace('.mac', '.sh')
                sh_path     = os.path.join(mac_sh_dir, sh_filename)
                with open(sh_path, 'w') as sh_file:
                    sh_file.write("#!/bin/bash\n")
                    sh_file.write("#PBS -q BS\n")
                    sh_file.write(f"cd {build_dir}\n")
                    sh_file.write(f"./SPECT2D {mac_path}\n")

                os.chmod(sh_path, 0o755)
                count += 1

    print(f"\nGenerated {count} job files in '{mac_sh_dir}'")

    # Submit jobs
    submitted = 0
    for filename in sorted(os.listdir(mac_sh_dir)):
        if not filename.endswith('.sh'):
            continue

        sh_path = os.path.join(mac_sh_dir, filename)
        print(f"Submitting {filename}...")

        # Throttle submissions
        while True:
            bln = submit_check("BS")
            myn = submit_check(os.getlogin())
            if bln < jobmax and myn < myjobmax:
                break
            print(f"Waiting: BS jobs={bln}/{jobmax}, user jobs={myn}/{myjobmax}")
            time.sleep(waittime)

        os.system(f"qsub {sh_path}")
        submitted += 1
        time.sleep(1)

    print(f"\nSubmitted {submitted} jobs to the queue")

if __name__ == "__main__":
    main()

