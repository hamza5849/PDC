from flask import Flask, request, jsonify
from flask_cors import CORS
import subprocess, time, re
import os
app = Flask(__name__, static_folder='frontend/public', static_url_path='')
CORS(app)

@app.route('/')
def index():
    return app.send_static_file('index.html')

@app.route('/api/run', methods=['POST'])
def run_array_sum():
    data = request.json
    np = int(data.get('np', 4))
    np = max(2, min(8, np))

    start = time.time()
    try:
        # Run the MPI Array Sum program
        result = subprocess.run(
            ['mpirun', '--allow-run-as-root', '--oversubscribe', '-np', str(np), './mapreduce'],
            capture_output=True, text=True, timeout=30
        )
        elapsed = round((time.time() - start) * 1000)
        output = result.stdout + result.stderr

        # Parse the total sum directly from the terminal output (much more reliable!)
        total_sum = 0
        match = re.search(r'Total Sum\s+:\s+(\d+)', output)
        if match:
            total_sum = int(match.group(1))

        return jsonify({
            'success': True,
            'log': output,
            'total_sum': total_sum,
            'elapsed': elapsed,
            'np': np
        })
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)})
if __name__ == '__main__':
    port = int(os.environ.get('PORT', 5000))
    app.run(host='0.0.0.0', port=port, debug=False)