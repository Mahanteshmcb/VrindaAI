from src.engines.blender_engine import create_blender_engine

try:
    engine = create_blender_engine()
    print(f"✅ Found Blender at: {engine.blender_path}")
    
    # Test a simple render
    spec = {
        "description": "Test Render",
        "quality": "low", # Fast render
        "duration": 1 # 1 second
    }
    print("🚀 Starting Test Render...")
    result = engine.render_from_spec(spec, "output/test_render")
    print(result)

except Exception as e:
    print(f"❌ Error: {e}")