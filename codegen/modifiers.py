from dataclasses import dataclass

class modifier:
    def __init__(self,
                 optional: str = None,
                 fixedLen: str = None,
                 varLen: str = None,
                 rename: str = None):

        self.optional = optional
        self.fixedLen = fixedLen
        self.varLen = varLen
        self.rename = rename
