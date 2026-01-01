# In __init__, below btn_build
        self.btn_render = QPushButton("🎨 GENERATE RENDER")
        self.btn_render.setFixedHeight(40)
        self.btn_render.setStyleSheet("background-color: #444; color: white; border: 1px solid #666;")
        self.btn_render.clicked.connect(self.generate_render)
        left_layout.addWidget(self.btn_render)