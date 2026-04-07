from classes import *

# section names
sections = ['.text', '.rodata', '.data', '.bss']

def parse_map(mapPath):
    # open map file
    mapFile = open(mapPath)

    # list to store
    memoryMapList = []

    for line in mapFile.readlines():
        # strip leading and trailing edge spaces
        line = line.strip()
        # split the line into word with a space delimiter
        splitLine = line.split()

        # add to memory map list
        if len(splitLine) == 4 and splitLine[0] in sections:    # memory map line check
            mapFileName = splitLine[3].split('/')[-1]  # file name parse
            memMap = MemoryMap(splitLine[0], splitLine[1], splitLine[2], mapFileName, None)
            memoryMapList.append(memMap)

    # close map file and return
    mapFile.close()
    return memoryMapList


def parse_make_output(makeOutputPath):
    # open map file
    makeOutputFile = open(makeOutputPath)

    # list to store
    swComponentList = []

    for line in makeOutputFile.readlines():
        # strip leading and trailing edge spaces
        line = line.strip()
        # split the line into word with a space delimiter
        splitLine = line.split()

        if splitLine: 
            subPath = splitLine[1].split('sources/')
            splitPath = subPath[1].split('/')
            splitPath = list(filter(None, splitPath))  # delete empty string

            # add to S/W component list
            if len(splitPath) == 2:
                swComponent = SwComponent(
                    splitPath[0], None, splitPath[-1].split('.')[0])
            else:
                swComponent = SwComponent(
                    splitPath[0], splitPath[1], splitPath[-1].split('.')[0])

            swComponentList.append(swComponent)

    # close make output file and return
    makeOutputFile.close()
    return swComponentList

# group obj files into list and calculate memory size of each group and subGroup
def group_obj(memoryMapList, swComponentList, groupInfoList, subGroupInfoList):
    for memMap in memoryMapList:
        # match swComponent
        for swComponent in swComponentList:
            if memMap.fileName.split('.')[0] == swComponent.name:
                memMap.swComponent = swComponent

        if not memMap.swComponent:
            etcSwComponent = SwComponent('etc', None, None)
            memMap.swComponent = etcSwComponent

        if memMap.swComponent.group == 'sal' and not memMap.swComponent.subGroup:
            memMap.swComponent.subGroup = 'salDefinition'            

    for memMap in memoryMapList:
        # calculate memory size of groups and set on list
        memMapGroupInfo = {'groupName': memMap.swComponent.group, '.text': 0, 
                            '.rodata': 0, '.data': 0, '.bss': 0}

        # if groupInfoList is None,
        if not groupInfoList:
            groupInfoList.append(memMapGroupInfo)
        
        memSizeAppendFlag = False
        for gInfo in groupInfoList:
            if gInfo['groupName'] == memMap.swComponent.group:
                # group memory size
                if 'text' in memMap.section:
                    gInfo['.text'] = gInfo['.text'] + int(memMap.objSize, 16)
                    memSizeAppendFlag = True
                elif 'rodata' in memMap.section:
                    gInfo['.rodata'] = gInfo['.rodata'] + int(memMap.objSize, 16)
                    memSizeAppendFlag = True
                elif 'data' in memMap.section:
                    gInfo['.data'] = gInfo['.data'] + int(memMap.objSize, 16)
                    memSizeAppendFlag = True
                elif 'bss' in memMap.section:
                    gInfo['.bss'] = gInfo['.bss'] + int(memMap.objSize, 16)
                    memSizeAppendFlag = True
                else:
                    pass
                break

        # if memMap's group is not added in groupInfoList,
        if not memSizeAppendFlag:
            # set section size
            if 'text' in memMap.section:
                memMapGroupInfo['.text'] = int(memMap.objSize, 16)
            elif 'rodata' in memMap.section:
                memMapGroupInfo['.rodata'] + int(memMap.objSize, 16)
            elif 'data' in memMap.section:
                memMapGroupInfo['.data'] + int(memMap.objSize, 16)
            elif 'bss' in memMap.section:
                memMapGroupInfo['.bss'] + int(memMap.objSize, 16)
            else:
                pass
            # add in groupInfoList
            groupInfoList.append(memMapGroupInfo)

        # calculate memory size of subGroups and set on list
        if not memMap.swComponent.group == 'etc':
            memMapSubGroupInfo = {'groupName': memMap.swComponent.group, 'subGroupName': memMap.swComponent.subGroup,
                                    '.text': 0, '.rodata': 0, '.data': 0, '.bss': 0}

            # if subGroupInfoList is None,
            if not subGroupInfoList:
                subGroupInfoList.append(memMapSubGroupInfo)

            memSizeAppendFlag = False
            for sInfo in subGroupInfoList:
                if sInfo['subGroupName'] == memMapSubGroupInfo['subGroupName']:
                    # sub component memory size
                    if 'text' in memMap.section:
                        sInfo['.text'] = sInfo['.text'] + int(memMap.objSize, 16)
                        memSizeAppendFlag = True
                    elif 'rodata' in memMap.section:
                        sInfo['.rodata'] = sInfo['.rodata'] + int(memMap.objSize, 16)
                        memSizeAppendFlag = True
                    elif 'data' in memMap.section:
                        sInfo['.data'] = sInfo['.data'] + int(memMap.objSize, 16)
                        memSizeAppendFlag = True
                    elif 'bss' in memMap.section:
                        sInfo['.bss'] = sInfo['.bss'] + int(memMap.objSize, 16)
                        memSizeAppendFlag = True
                    else:
                        pass
                    break

            # if memMap's subGroup is not added in subGroupInfoList,
            if not memSizeAppendFlag:
                # set section size
                if 'text' in memMap.section:
                    memMapSubGroupInfo['.text'] = int(memMap.objSize, 16)
                elif 'rodata' in memMap.section:
                    memMapSubGroupInfo['.rodata'] + int(memMap.objSize, 16)
                elif 'data' in memMap.section:
                    memMapSubGroupInfo['.data'] + int(memMap.objSize, 16)
                elif 'bss' in memMap.section:
                    memMapSubGroupInfo['.bss'] + int(memMap.objSize, 16)
                else:
                    pass
                # add in subGroupInfoList
                subGroupInfoList.append(memMapSubGroupInfo)

    # add SAL Definition memory size to each SAL OS implementation memory size
    # for sInfo1 in subGroupInfoList:
    #     if 'sal_' in sInfo1['subGroupName']:
    #         if memMap.section in ramSection:
    #             for sInfo2 in subGroupInfoList:
    #                 if sInfo2['subGroupName'] == 'salDefinition':
    #                     sInfo1['subGroupRamSize'] = sInfo1['subGroupRamSize'] + sInfo2['subGroupRamSize']
    #         elif memMap.section in romSection:
    #             for sInfo2 in subGroupInfoList:
    #                 if sInfo2['subGroupName'] == 'salDefinition':
    #                     sInfo1['subGroupRomSize'] = sInfo1['subGroupRomSize'] + sInfo2['subGroupRomSize']
    #         else:
    #             None

    # print('########## Group Info ##########')
    # for g in groupInfoList:
    #     print(g)

    # print()
    # print('########## Sub Group Info ##########')
    # for s in subGroupInfoList:
    #     print(s)