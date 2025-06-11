import os
import subprocess
import sys
import glob

def main():
    # 1) Locate the directory containing .mac files
    script_dir = os.path.dirname(os.path.abspath(__file__))
    run_mac_dir = os.path.abspath(os.path.join(script_dir, "..", "run_mac_files_xy"))
    if not os.path.isdir(run_mac_dir):
        print(f"Error: directory not found: {run_mac_dir}")
        sys.exit(1)

    # 2) Find all .mac files
    mac_files = sorted(glob.glob(os.path.join(run_mac_dir, "*.mac")))
    if not mac_files:
        print(f"No .mac files found in {run_mac_dir}")
        sys.exit(0)

    # 3) Path to the SPECT_Simulation executable (must be in the same dir as this script)
    exe_path = os.path.join(script_dir, "SPECT_Simulation")
    if not os.path.isfile(exe_path) or not os.access(exe_path, os.X_OK):
        print(f"Error: Executable not found or not executable: {exe_path}")
        sys.exit(1)

    # 4) Loop over each .mac file, run and rename its output
    for mac in mac_files:
        mac_basename = os.path.basename(mac)                # e.g. "run_1.68.mac"
        name_without_ext = os.path.splitext(mac_basename)[0]  # e.g. "run_1.68"
        print(f"Running simulation with macro: {mac_basename}")

        # Invoke the simulation
        try:
            result = subprocess.run(
                [exe_path, mac],
                check=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE
            )
        except subprocess.CalledProcessError as e:
            print(f"Simulation failed for {mac_basename} (exit code {e.returncode})")
            print("--- stdout ---")
            print(e.stdout.decode("utf-8", errors="ignore"))
            print("--- stderr ---")
            print(e.stderr.decode("utf-8", errors="ignore"))
            sys.exit(e.returncode)

        # After a successful run, rename the generated root file:
        old_root = os.path.join(script_dir, "spect_simulation.root")
        if not os.path.isfile(old_root):
            print(f"Warning: expected output file not found: {old_root}")
        else:
            new_root = os.path.join(script_dir, f"spect_simulation_{name_without_ext}.root")
            # If the target already exists, overwrite it
            if os.path.isfile(new_root):
                os.remove(new_root)
            os.rename(old_root, new_root)
            print(f"  Renamed output to: {os.path.basename(new_root)}")

        print(f"  {mac_basename} completed successfully.\n")

    print("All simulations finished.")

if __name__ == "__main__":
    main()