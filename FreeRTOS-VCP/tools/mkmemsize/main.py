# import modules
import os

from classes import *
from parse_file import *  # for parse files and classify objs
from generate_excel import *  # To make parsing result an Excel file

def main():
    # set map file directory
    # baseDir = 'Z:/workspace/TCC70xx/build/tcc70xx/gcc/tcc70xx-freertos-debug' # for run in window 
    baseDir = './tcc70xx-freertos-debug' # for run in development server
    mapFileName = 'boot.map'
    mapPath = os.path.join(baseDir, mapFileName)

    # parse map file
    memoryMapList = parse_map(mapPath)

    # set make output file directory
    makeOutputFileName = 'compile_console_output.log'
    makeOutputPath = os.path.join(baseDir, makeOutputFileName)

    # parse make output
    swComponentList = parse_make_output(makeOutputPath)

    # group obj files and calculate each memory size
    groupInfoList = []
    subGroupInfoList = []
    group_obj(memoryMapList, swComponentList,
              groupInfoList, subGroupInfoList)

    # print()
    # print('########### memoryMapList ############')
    # for m in memoryMapList:
    #     print(m)

    # print()
    # print('########### groupInfoList ############')
    # for g in groupInfoList:
    #     print(g)

    # print()
    # print('########### subGroupInfoList ############')
    # for s in subGroupInfoList:
    #     print(s)

    # make parsing result an Excel file
    # excelFilePath = 'Z:/workspace/TCC70xx/mcu_bsp/build/tcc70xx/gcc/output/sectionMemoryInfo.xlsx' # set output Excel file path, for run in window
    excelFilePath = './output/sectionMemoryInfo.xlsx' # set output Excel file path, for run in development server
    generate_excel(excelFilePath, memoryMapList, groupInfoList, subGroupInfoList)
    generate_group_info_sheet(excelFilePath, groupInfoList)
    generate_subGroup_info_sheet(excelFilePath, subGroupInfoList)

if __name__ == "__main__":
    main()
