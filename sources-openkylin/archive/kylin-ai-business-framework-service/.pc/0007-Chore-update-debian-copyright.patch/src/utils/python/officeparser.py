# This Python file uses the following encoding: utf-8

from abc import ABC, abstractmethod

class OfficeParser(ABC):
    @abstractmethod
    def contents(self):
        pass

    @abstractmethod
    def title(self):
        pass

    @abstractmethod
    def author(self):
        pass

    @abstractmethod
    def pages(self):
        pass
