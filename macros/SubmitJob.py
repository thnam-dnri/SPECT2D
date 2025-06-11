#!/usr/bin/env python3
# coding:utf-8
import subprocess
import time
import os

# Configuration
jobmax    = 70     # max jobs allowed in queue "BS"
myjobmax  = 50     # max jobs allowed for user (e.g., "thnam")
waittime  = 30     # seconds to wait between queue checks
startnum  = 0      # start index of scripts to submit
stopnum   = 10000  # end index of scripts to submit


def submit_check(keyword):
    """
    Returns the number of lines from `qstat` containing the given keyword.
    """
    # Run the pipeline: qstat | grep keyword
    try:
        output = subprocess.check_output(
            f"qstat | grep {keyword}", shell=True,
            universal_newlines=True,
        )
        # Count non-empty lines
        lines = [line for line in output.splitlines() if line.strip()]
        return len(lines)
    except subprocess.CalledProcessError:
        # grep returns exit code 1 if no matches
        return 0


def main():
    directory = "./mac_sh"
    files = os.listdir(directory)
    files.sort()

    print("Found files:", files)
    count = 0

    for fname in files:
        if not fname.endswith('.sh'):
            continue
        if count < startnum or count > stopnum:
            count += 1
            continue

        script_path = os.path.join(directory, fname)
        print(f'"{fname}" trying to start (index {count})')

        # Check queue statuses
        bln = submit_check("BS")
        myn = submit_check(os.getlogin())

        # Wait until below thresholds
        while bln >= jobmax or myn >= myjobmax:
            print(f'"{fname}" is waiting: BS jobs={bln}/{jobmax}, my jobs={myn}/{myjobmax}')
            time.sleep(waittime)
            bln = submit_check("BS")
            myn = submit_check(os.getlogin())

        # Submit the shell script
        os.system(f"qsub {script_path}")
        time.sleep(1)
        count += 1

    print("fin")

if __name__ == '__main__':
    main()

