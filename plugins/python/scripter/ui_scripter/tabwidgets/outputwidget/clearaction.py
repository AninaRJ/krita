"""
SPDX-FileCopyrightText: 2017 Eliakin Costa <eliakim170@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
"""
try:
    from PyQt6.QtGui import QAction#, QIcon
except:
    from PyQt5.QtWidgets import QAction
    # from PyQt5.QtGui import QIcon
from builtins import i18n


class ClearAction(QAction):

    def __init__(self, scripter, toolbar, parent=None):
        super(ClearAction, self).__init__(parent)
        self.scripter = scripter
        self.toolbar = toolbar

        self.triggered.connect(self.clear)

        self.setText(i18n("Clear"))
        # path to the icon
        # self.setIcon(QIcon(':/icons/clear.svg'))

    def clear(self):
        self.toolbar.outputtextedit.clear()
