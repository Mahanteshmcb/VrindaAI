# To start in cmd prompt
# cd C:\Users\Mahantesh\DevelopmentProjects\VrindaAI\VrindaAI
# python memory_service.py
from flask import Flask, request, jsonify
import chromadb
import uuid
import logging

# --- Setup ---
# Disable excessive logging from underlying libraries
logging.getLogger('werkzeug').setLevel(logging.WARNING)
logging.getLogger('sentence_transformers').setLevel(logging.WARNING)

# Initialize Flask app
app = Flask(__name__)

# --- ChromaDB Initialization ---
try:
    # This will create a local, persistent database in a folder named 'chroma_db'
    client = chromadb.PersistentClient(path="./chroma_db")

    # This model runs locally and doesn't require an internet connection after its first download.
    collection = client.get_or_create_collection(name="vrinda_memory")

    print("--------------------------------------------------")
    print("✅ ChromaDB Memory Service Initialized Successfully.")
    print("--------------------------------------------------")
except Exception as e:
    print(f"❌ Error initializing ChromaDB: {e}")
    client = None

# --- API Endpoints ---
@app.route('/add_memory', methods=['POST'])
def add_memory():
    """Receives text, converts it to an embedding, and stores it."""
    if not client:
        return jsonify({"status": "error", "message": "ChromaDB not initialized"}), 500

    data = request.get_json()
    if not data or 'text' not in data:
        return jsonify({"status": "error", "message": "Invalid request, 'text' field is required."}), 400

    try:
        memory_text = data['text']
        # Each memory needs a unique ID.
        memory_id = str(uuid.uuid4())

        collection.add(
            documents=[memory_text],
            ids=[memory_id]
        )
        print(f"🧠 Memory Added: '{memory_text}'")
        return jsonify({"status": "success", "id": memory_id})
    except Exception as e:
        print(f"❌ Error in /add_memory: {e}")
        return jsonify({"status": "error", "message": str(e)}), 500

@app.route('/query_memory', methods=['POST'])
def query_memory():
    """Receives a query, finds the most similar memories, and returns them."""
    if not client:
        return jsonify({"status": "error", "message": "ChromaDB not initialized"}), 500

    data = request.get_json()
    if not data or 'query' not in data:
        return jsonify({"status": "error", "message": "Invalid request, 'query' field is required."}), 400

    try:
        query_text = data['query']
        num_results = data.get('n_results', 3) # Allow specifying number of results, default to 3

        # Find the most similar memories.
        results = collection.query(
            query_texts=[query_text],
            n_results=num_results,
            include=["documents", "distances"]
        )

        # --- NEW: Combine documents and distances into a list of objects ---
        output_results = []
        if results is not None:
            # Using .get() prevents "None is not subscriptable" errors
            documents_list = results.get('documents')
            distances_list = results.get('distances')
            
            # Check if lists exist and are not empty
            if documents_list and len(documents_list) > 0 and documents_list[0] is not None:
                documents = documents_list[0]
                # Safely handle distances; fallback to 0.0 if missing
                distances = distances_list[0] if (distances_list and len(distances_list) > 0) else []
                
                for i in range(len(documents)):
                    dist_val = distances[i] if i < len(distances) else 0.0
                    output_results.append({"document": documents[i], "distance": dist_val})

        print(f"🔍 Query: '{query_text}' -> Found: {output_results}")
        return jsonify({"status": "success", "results": output_results})
    except Exception as e:
        print(f"❌ Error in /query_memory: {e}")
        return jsonify({"status": "error", "message": str(e)}), 500

if __name__ == '__main__':
    print("🚀 Starting VrindaAI Memory Service...")
    # Run the service on a local port. host='127.0.0.1' ensures it's only accessible locally.
    app.run(host='127.0.0.1', port=5100)
