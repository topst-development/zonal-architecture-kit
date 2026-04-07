from dataclasses import dataclass

# define S/W component dataclass
@dataclass
class SwComponent:
    def __init__(
        self, group: str, subGroup: str, name: str
    ) -> None:
        self.group = group
        self.subGroup = subGroup
        self.name = name

    def __repr__(self):
        return (
            self.__class__.__qualname__ +
            f"(group={self.group!r}, subGroup={self.subGroup!r}, "
            f"name={self.name!r})"
        )

# define memory map dataclass
@dataclass
class MemoryMap(SwComponent):
    def __init__(
        self, section: str, address: str, objSize: str, fileName: str, swComponent: SwComponent
    ) -> None:
        self.section = section
        self.address = address
        self.objSize = objSize
        self.fileName = fileName
        self.swComponent = swComponent

    def __repr__(self):
        return (
            self.__class__.__qualname__ +
            f"(section={self.section!r}, address={self.address!r}, "
            f"objSize={self.objSize!r}, fileName={self.fileName!r}, swComponent={self.swComponent!r})"
        )
