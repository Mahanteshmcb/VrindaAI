import sys
import os
import json
from pathlib import Path
from PyQt6.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout, 
                             QHBoxLayout, QTextEdit, QPushButton, QLabel, 
                             QFrame, QSplitter, QStatusBar)
from PyQt6.QtCore import Qt, QThread, pyqtSignal
from PyQt6.QtGui import QFont, QColor

# --- 3D VISUALIZATION IMPORTS ---
import pyvista as pv
from pyvistaqt import QtInteractor

# --- VRINDA CORE IMPORTS ---
current_dir = Path(__file__).parent
src_dir = current_dir.parent
sys.path.append(str(src_dir.parent)) # Root of VrindaAI

from src.core.orchestrator import create_orchestrator
# Import the new Blender Engine factory
from src.engines.blender_engine import create_blender_engine

# --- WORKER THREADS ---

class EngineWorker(QThread):
    """Handles communication with Vryndara Kernel (Geometry Generation)"""
    finished = pyqtSignal(dict)
    log = pyqtSignal(str)

    def __init__(self, prompt):
        super().__init__()
        self.prompt = prompt
        self.orchestrator = create_orchestrator()

    def run(self):
        self.log.emit(f"🚀 Sending task to Vryndara: {self.prompt}")
        task = {"description": self.prompt, "engine": "unreal", "type": "game"}
        try:
            result = self.orchestrator.execute_workflow(task)
            self.finished.emit(result)
        except Exception as e:
            self.finished.emit({"status": "error", "error": str(e)})

class RenderWorker(QThread):
    """Handles communication with Blender Engine (Photorealistic Rendering)"""
    finished = pyqtSignal(dict)
    log = pyqtSignal(str)

    def __init__(self, stl_path):
        super().__init__()
        self.stl_path = stl_path
        self.engine = create_blender_engine()

    def run(self):
        self.log.emit("⏳ Setting up studio lighting & materials...")
        
        # Define output
        output_dir = str(Path(self.stl_path).parent / "render_output")
        
        # Create the render spec for the engine
        spec = {
            "description": "Product Shot",
            "quality": "medium", 
            "duration": 2,       
            "assets": [self.stl_path] 
        }
        
        self.log.emit("🚀 Rendering frames (this may take a minute)...")
        # Use the robust pipeline method
        result = self.engine.render_from_spec(spec, output_dir)
        self.finished.emit(result)

# --- MAIN DASHBOARD ---

class VrindaDashboard(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("VRINDA-AI // ENGINEERING CONSOLE // 3D MODE")
        self.resize(1200, 800)
        self.apply_dark_theme()
        
        main_widget = QWidget()
        self.setCentralWidget(main_widget)
        main_layout = QHBoxLayout(main_widget)
        
        # --- LEFT PANEL (Controls) ---
        left_panel = QFrame()
        left_layout = QVBoxLayout(left_panel)
        left_panel.setFixedWidth(350)
        
        title = QLabel("COMMAND LINK")
        title.setFont(QFont("Segoe UI", 12, QFont.Weight.Bold))
        left_layout.addWidget(title)
        
        self.prompt_input = QTextEdit()
        self.prompt_input.setPlaceholderText("Enter Task (e.g., 'Solid 40mm Cube')...")
        self.prompt_input.setFixedHeight(100)
        self.prompt_input.setStyleSheet("background-color: #2b2b2b; color: #00ffcc; border: 1px solid #444;")
        left_layout.addWidget(self.prompt_input)
        
        # Build Button
        self.btn_build = QPushButton("⚡ BUILD ARTIFACT")
        self.btn_build.setFixedHeight(40)
        self.btn_build.setStyleSheet("background-color: #007acc; color: white; font-weight: bold;")
        self.btn_build.clicked.connect(self.start_engine)
        left_layout.addWidget(self.btn_build)

        # Render Button (Initially Disabled)
        self.btn_render = QPushButton("🎨 GENERATE RENDER")
        self.btn_render.setFixedHeight(40)
        self.btn_render.setStyleSheet("background-color: #444; color: white; border: 1px solid #666;")
        self.btn_render.setEnabled(False)
        self.btn_render.clicked.connect(self.generate_render)
        left_layout.addWidget(self.btn_render)
        
        left_layout.addWidget(QLabel("SYSTEM LOGS:"))
        self.log_viewer = QTextEdit()
        self.log_viewer.setReadOnly(True)
        self.log_viewer.setStyleSheet("font-family: Consolas; font-size: 10px; color: #aaa;")
        left_layout.addWidget(self.log_viewer)
        
        # --- RIGHT PANEL (3D Viewer) ---
        right_panel = QFrame()
        right_layout = QVBoxLayout(right_panel)
        
        # 3D WIDGET
        self.plotter = QtInteractor(right_panel)
        self.plotter.set_background("#001a33") 
        self.plotter.add_text("AWAITING GEOMETRY", position='upper_left', font_size=10, color='white')
        self.plotter.add_axes()
        right_layout.addWidget(self.plotter.interactor)
        
        # Data Grid
        data_frame = QFrame()
        data_frame.setStyleSheet("background-color: #222; border-radius: 8px;")
        data_layout = QHBoxLayout(data_frame)
        self.lbl_mass = self.create_stat_box("MASS", "--- g")
        self.lbl_vol = self.create_stat_box("VOLUME", "--- cm³")
        self.lbl_cost = self.create_stat_box("EST. COST", "$---")
        self.lbl_mat = self.create_stat_box("MATERIAL", "---")
        
        data_layout.addLayout(self.lbl_mass)
        data_layout.addLayout(self.lbl_vol)
        data_layout.addLayout(self.lbl_cost)
        data_layout.addLayout(self.lbl_mat)
        right_layout.addWidget(data_frame)
        
        main_layout.addWidget(left_panel)
        main_layout.addWidget(right_panel)
        
        self.status = QStatusBar()
        self.setStatusBar(self.status)

    def create_stat_box(self, title, value):
        layout = QVBoxLayout()
        lbl_title = QLabel(title)
        lbl_title.setStyleSheet("color: #888; font-size: 10px;")
        lbl_val = QLabel(value)
        lbl_val.setStyleSheet("color: white; font-size: 16px; font-weight: bold;")
        layout.addWidget(lbl_title)
        layout.addWidget(lbl_val)
        return layout

    def start_engine(self):
        prompt = self.prompt_input.toPlainText()
        if not prompt: return
        self.btn_build.setEnabled(False)
        self.btn_render.setEnabled(False)
        self.log_viewer.clear()
        
        self.worker = EngineWorker(prompt)
        self.worker.log.connect(self.log_viewer.append)
        self.worker.finished.connect(self.on_task_complete)
        self.worker.start()

    def on_task_complete(self, result):
        self.btn_build.setEnabled(True)
        if result.get("status") == "offloaded":
            data = result.get("output", {}).get("blueprint_data", {})
            self.update_stat(self.lbl_mass, f"{data.get('mass_grams', 'ERR')} g")
            self.update_stat(self.lbl_vol, f"{data.get('volume_cm3', 'ERR')} cm³")
            self.update_stat(self.lbl_cost, data.get('cost_est', '$0.00'))
            self.update_stat(self.lbl_mat, data.get('material', 'Unknown'))
            
            # --- LOAD 3D MESH ---
            stl_path = result.get("output", {}).get("high_res")
            if stl_path and os.path.exists(stl_path):
                # STORE PATH FOR RENDERER
                self.current_stl_path = stl_path 
                self.btn_render.setEnabled(True) # Enable render button
                
                self.plotter.clear() 
                self.plotter.add_mesh(pv.read(stl_path), color="cyan", show_edges=True, opacity=0.8)
                self.plotter.reset_camera()
                self.plotter.add_text("ENGINEERING SCHEMATIC // LIVE RENDER", position='upper_left', font_size=10, color='cyan')
            else:
                self.log_viewer.append("⚠️ STL file not found.")
        else:
            self.log_viewer.append(f"❌ Task Failed: {result}")

    def generate_render(self):
        # 1. Check if we have a model to render
        if not hasattr(self, 'current_stl_path') or not os.path.exists(self.current_stl_path):
            self.log_viewer.append("⚠️ No Geometry to Render. Build an artifact first.")
            return

        self.log_viewer.append("🎨 Initializing Blender Engine...")
        self.btn_render.setEnabled(False) # Prevent double-clicking

        # 2. Run in a separate thread
        self.render_worker = RenderWorker(self.current_stl_path)
        self.render_worker.log.connect(self.log_viewer.append)
        self.render_worker.finished.connect(self.on_render_complete)
        self.render_worker.start()

    def on_render_complete(self, result):
        self.btn_render.setEnabled(True)
        if result.get('status') == 'success':
            self.log_viewer.append("✅ Render Complete! Check 'render_output' folder.")
        else:
            self.log_viewer.append(f"❌ Render Failed: {result.get('error')}")

    def update_stat(self, layout, value):
        layout.itemAt(1).widget().setText(value)

    def apply_dark_theme(self):
        self.setStyleSheet("""
            QMainWindow { background-color: #1e1e1e; } 
            QLabel { color: #e0e0e0; }
            QTextEdit { background-color: #2b2b2b; color: #ddd; border: 1px solid #444; }
        """)

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = VrindaDashboard()
    window.show()
    sys.exit(app.exec())