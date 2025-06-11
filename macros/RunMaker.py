#!/usr/bin/env python3
import os

def main():
    # Scan parameters
    step_size = 1.68  # mm
    x_steps = 40
    y_steps = 20
    z_offset = 0.0  # mm

    # Output directory for .mac and .sh files
    output_dir = "mac_sh"
    os.makedirs(output_dir, exist_ok=True)

    # Generate symmetric position lists
    x_positions = [(-step_size * (x_steps - 1) / 2.0 + i * step_size) for i in range(x_steps)]
    y_positions = [(-step_size * (y_steps - 1) / 2.0 + j * step_size) for j in range(y_steps)]

    count = 0
    for x in x_positions:
        for y in y_positions:
            # Format values
            x_str = f"{x:.2f}"
            y_str = f"{y:.2f}"
            z_str = f"{z_offset:.1f}"

            # Build .mac filename and path
            mac_filename = f"run_x_{x_str}_y_{y_str}_zoff_{z_str}.mac"
            mac_path = os.path.join(output_dir, mac_filename)

            # Write macro file
            with open(mac_path, 'w') as mac_file:
                mac_file.write(f"/simulation/offset  {x_str}  {y_str}  {z_str} mm\n")
                mac_file.write("/run/initialize\n")
                mac_file.write("/run/beamOn        1000000\n")

            # Build .sh filename and path
            sh_filename = mac_filename.replace('.mac', '.sh')
            sh_path = os.path.join(output_dir, sh_filename)

            # Write shell script
            with open(sh_path, 'w') as sh_file:
                sh_file.write("#! /bin/bash\n")
                sh_file.write("#PBS -q BS\n\n")
                sh_file.write("cd ~/Geant4/SPEC2D/build/\n")
                sh_file.write(f"./SPECT2D ../{output_dir}/{mac_filename}\n")

            # Make the shell script executable
            os.chmod(sh_path, 0o755)

            print(f"Created {mac_path} and {sh_path}")
            count += 1

    print(f"\nGenerated {count} .mac and .sh files in '{output_dir}' directory.")

if __name__ == "__main__":
    main()

