#!/usr/bin/env python

import io
import sys
import json
import shutil
import typing
import os.path
import argparse
import subprocess
from zipfile import ZipFile
from distutils.dir_util import copy_tree

import re
from os import listdir
from os.path import isfile, join


def main():
    ap = argparse.ArgumentParser(
        description='auto generate LVGL font files from fonts')
    ap.add_argument('--config', '-c', type=str,
                    action='append', help='config file to use')
    ap.add_argument('--obsolete', type=str, help='List of obsolete files')
    ap.add_argument('--output', type=str, help='output file name')
    args = ap.parse_args()

    # for config_file in args.config:
    #     if not os.path.exists(config_file):
    #         sys.exit(f'Error: the config file {config_file} does not exist.')
    #     if not os.access(config_file, os.R_OK):
    #         sys.exit(
    #             f'Error: the config file {config_file} is not accessible (permissions?).')

    if args.obsolete:
        obsolete_file_path = os.path.join(
            os.path.dirname(sys.argv[0]), args.obsolete)
        if not os.path.exists(obsolete_file_path):
            sys.exit(
                f'Error: the "obsolete" file {args.obsolete} does not exist.')
        if not os.access(obsolete_file_path, os.R_OK):
            sys.exit(
                f'Error: the "obsolete" file {args.obsolete} is not accessible (permissions?).')

    source = "/sources"
    try:
        onlyfiles = [f for f in listdir(source)]
    except:
        source = "../../InfiniTime"
        # onlyfiles = [f for f in listdir(resourcesPath)]
    # print("resources files: " + resourcesPath)
    # print(onlyfiles)
    resourcesPath = source + "/src/resources"

    zipPath = source + "/build/src/resources/" + args.output
    print("ZipFile: " + zipPath)

    try:
        os.remove(zipPath)
    except OSError as error:
        # print("os.remove OSError:")
        # print(error)
        pass

    zf = ZipFile(zipPath, mode='w')
    resource_files = [
        {
            "filename": "clocks.txt",
            "path": "/clocks.txt"
        }
    ]

    # SCHEDULES:
    schedules = [
        {
            "midday": "01111100",  # midday,M,T,W,T,F,S,S
            "slices": [
                [30, 31, 50, 50, 50, 0],
                [36, 38, 50, 50, 50, 0],
                [42, 45, 50, 50, 50, 0],
                [48, 52, 50, 50, 50, 0],
                [54, 59, 50, 50, 50, 0],
                [60, 210, 50, 50, 50, 1],  # "sleep"
                #
                [210, 232, 231, 46, 52, 2],  # 7 - 7.75 , "bowl"
                [232, 250, 255, 199, 0, 3],  # 7.75 - 8.33 , "clothes"
                [250, 270, 91, 185, 1, 4],  # 8.33 - 9 , "bus"
                [270, 360, 2, 113, 255, 5]  # 9 - 12 , "school"
            ]
        },
        {
            "midday": "11111100",  # midday,M,T,W,T,F,S,S
            "slices": [
                [30, 31, 50, 50, 50, 0],
                [36, 38, 50, 50, 50, 0],
                [42, 45, 50, 50, 50, 0],
                [48, 52, 50, 50, 50, 0],
                [54, 59, 50, 50, 50, 0],
                [60, 210, 50, 50, 50, 1],  # "sleep"
                #
                [210, 232, 231, 46, 52, 2],  # 7 - 7.75 , "bowl"
                [232, 250, 255, 199, 0, 3],  # 7.75 - 8.33 , "clothes"
                [250, 270, 91, 185, 1, 4],  # 8.33 - 9 , "bus"
                [270, 360, 2, 113, 255, 5]  # 9 - 12 , "school"
            ]
        }
    ]
    clocks_bytes = []
    for i, schedule in enumerate(schedules):
        mid = int(schedule["midday"], 2).to_bytes(
            (len(schedule["midday"]) + 7) // 8, byteorder='big')
        clocks_bytes += [mid[0], i]

        clock_bytes = []
        for row in schedule["slices"]:
            clock_bytes.append(round(row[0] * 255 / 360))
            clock_bytes.append(round(row[1] * 255 / 360))
            clock_bytes.append(row[2])
            clock_bytes.append(row[3])
            clock_bytes.append(row[4])
            clock_bytes.append(row[5])
        zf.writestr("schedule_" + str(i) + ".bin", bytes(clock_bytes))

    zf.writestr("schedules.bin", bytes(clocks_bytes))

    #

    imgfiles = [f for f in listdir(os.path.join(resourcesPath, "images"))]
    print("imgfiles")
    print(imgfiles)

    for imgfilename in imgfiles:
        name = re.sub(r"\.png", "", imgfilename)
        binFilename = name + '.bin'
        resource_files.append({
            "filename": binFilename,
            "path": "/images/" + binFilename
        })
        binPath = source + "/src/resources/" + binFilename
        # print("binPath:")
        # print(binPath)
        # print(binFilename)
        zf.write(binPath, binFilename)

    # for config_file in args.config:
    #     with open(config_file, 'r') as fd:
    #         data = json.load(fd)

    #     resource_names = set(data.keys())
    #     for name in resource_names:
    #         resource = data[name]
    #         binFilename = name + '.bin'

    #         resource_files.append({
    #             "filename": binFilename,
    #             "path": resource['target_path'] + binFilename
    #         })

    #         if not os.path.exists(binFilename):
    #             binFilename = os.path.join(os.path.dirname(sys.argv[0]), binFilename)

    #         try:
    #             zf.write(binFilename)
    #         except:
    #             print("!zf.write(binFilename)")

    if args.obsolete:
        obsolete_file_path = os.path.join(
            os.path.dirname(sys.argv[0]), args.obsolete)
        with open(obsolete_file_path, 'r') as fd:
            obsolete_data = json.load(fd)
    else:
        obsolete_data = {}

    output = {
        'resources': resource_files,
        'obsolete_files': obsolete_data
    }

    with open(source + "/src/resources/resources.json", 'w') as fd:
        json.dump(output, fd, indent=4)

    zf.write(source + '/src/resources/resources.json', "resources.json")
    zf.write(source + '/src/resources/clocks.txt', "clocks.txt")
    zf.close()

    print("resource_files:")
    print(resource_files)


if __name__ == '__main__':
    main()
