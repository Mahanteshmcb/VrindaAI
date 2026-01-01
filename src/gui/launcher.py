import sys
import os
from pathlib import Path
from PyQt6.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout, 
                             QHBoxLayout, QPushButton, QLabel, QFrame, QStackedWidget)
from PyQt6.QtCore import Qt
from PyQt6.QtGui import QFont, QIcon

# Import the Engineering App (The code we just perfected)
# Ensure you moved dashboard.py to apps/engineering_app.py and changed class name to 'EngineeringApp'
# Or just import the class directly if you kept it as VrindaDashboard
sys.path.append(str(Path(__file__).parent.parent.parent)) # Root
from src.gui.dashboard import VrindaDashboard 

class SidebarButton(QPushButton):
    def __init__(self, text, icon_char, callback):
        super().__init__(f" {icon_char}   {text}")
        self.setFixedHeight(50)
        self.setStyleSheet("""
            QPushButton {
                text-align: left;
                padding-left: 20px;
                border: none;
                color: #888;
                font-size: 14px;
                font-family: 'Segoe UI';
                background-color: transparent;
            }
            QPushButton:hover {
                background-color: #2a2a2a;
                color: white;
                border-left: 3px solid #00ffcc;
            }
            QPushButton:checked {
                color: #00ffcc;
                font-weight: bold;
            }
        """)
        self.clicked.connect(callback)

class VryndaraLauncher(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("VRYNDARA // PLATFORM ONE")
        self.resize(1400, 900)
        self.setStyleSheet("background-color: #121212;")

        # --- MAIN LAYOUT ---
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        main_layout = QHBoxLayout(central_widget)
        main_layout.setContentsMargins(0, 0, 0, 0)
        main_layout.setSpacing(0)

        # --- LEFT SIDEBAR (The Dock) ---
        sidebar = QFrame()
        sidebar.setFixedWidth(250)
        sidebar.setStyleSheet("background-color: #1e1e1e; border-right: 1px solid #333;")
        sidebar_layout = QVBoxLayout(sidebar)
        sidebar_layout.setContentsMargins(0, 0, 0, 0)
        sidebar_layout.setSpacing(5)

        # Logo Area
        lbl_logo = QLabel("VRYNDARA")
        lbl_logo.setFixedHeight(80)
        lbl_logo.setAlignment(Qt.AlignmentFlag.AlignCenter)
        lbl_logo.setStyleSheet("color: white; font-size: 20px; font-weight: bold; letter-spacing: 4px;")
        sidebar_layout.addWidget(lbl_logo)

        # App Buttons
        self.btn_home = SidebarButton("HOME", "🏠", lambda: self.switch_tab(0))
        self.btn_eng = SidebarButton("ENGINEERING", "⚙️", lambda: self.switch_tab(1))
        self.btn_rsrch = SidebarButton("RESEARCH", "🌐", lambda: self.switch_tab(2)) # Placeholder
        
        sidebar_layout.addWidget(self.btn_home)
        sidebar_layout.addWidget(self.btn_eng)
        sidebar_layout.addWidget(self.btn_rsrch)
        sidebar_layout.addStretch()

        # System Status
        status_frame = QFrame()
        status_frame.setStyleSheet("background-color: #252525; padding: 10px;")
        status_layout = QVBoxLayout(status_frame)
        status_layout.addWidget(QLabel("🟢 CORE: ONLINE"))
        status_layout.addWidget(QLabel("🧠 BRAIN: MISTRAL"))
        sidebar_layout.addWidget(status_frame)

        main_layout.addWidget(sidebar)

        # --- RIGHT CONTENT AREA (App Stack) ---
        self.stack = QStackedWidget()
        main_layout.addWidget(self.stack)

        # 1. Home Screen
        self.home_screen = self.create_home_screen()
        self.stack.addWidget(self.home_screen)

        # 2. Engineering App (VrindaAI)
        # We wrap the QMainWindow of the dashboard into a widget for the stack
        self.eng_app = VrindaDashboard() 
        # Note: Since VrindaDashboard is a Main Window, usually we embed its central widget, 
        # but for speed, adding it directly works in PyQt6 often, or we treat it as a popup.
        # Ideally, refactor VrindaDashboard to inherit QWidget. 
        # For now, let's just use its central widget logic:
        self.stack.addWidget(self.eng_app)

        # 3. Placeholder
        self.stack.addWidget(QLabel("Research Module Coming Soon...", alignment=Qt.AlignmentFlag.AlignCenter))

    def create_home_screen(self):
        widget = QWidget()
        layout = QVBoxLayout(widget)
        layout.setAlignment(Qt.AlignmentFlag.AlignCenter)
        
        lbl_welcome = QLabel("WELCOME TO VRYNDARA")
        lbl_welcome.setStyleSheet("font-size: 32px; color: #555; font-weight: bold;")
        layout.addWidget(lbl_welcome)
        
        lbl_sub = QLabel("Select a module from the sidebar to begin.")
        lbl_sub.setStyleSheet("font-size: 16px; color: #444;")
        layout.addWidget(lbl_sub)
        return widget

    def switch_tab(self, index):
        self.stack.setCurrentIndex(index)

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = VryndaraLauncher()
    window.show()
    sys.exit(app.exec())