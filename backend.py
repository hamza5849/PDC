from flask import Flask, request, jsonify
from flask_cors import CORS
import subprocess, time, re, os

app = Flask(__name__, static_folder='frontend/public', static_url_path='')
CORS(app)

@app.route('/')
def index():
    return app.send_static_file('index.html')

@app.route('/api/run', methods=['POST'])
def run_mapreduce():
    data = request.json
    np = int(data.get('np', 4))
    np = max(2, min(8, np))

    start = time.time()
    try:
        result = subprocess.run(
            ['mpirun', '--allow-run-as-root', '--oversubscribe', '-np', str(np), './mapreduce'],
            capture_output=True, text=True, timeout=60
        )
        elapsed = round((time.time() - start) * 1000)
        output = result.stdout + result.stderr

        # Parse output.txt written by mapreduce
        unique_words = 0
        exec_time = 0.0
        processes = np
        workers = np - 1
        throughput = 0.0
        top_words = []

        if os.path.exists('output.txt'):
            with open('output.txt', 'r') as f:
                lines = f.readlines()

            in_top = False
            for line in lines:
                line = line.strip()
                if line.startswith('Total Unique Words:'):
                    unique_words = int(line.split(':')[1].strip())
                elif line.startswith('Execution Time:'):
                    exec_time = float(line.split(':')[1].replace('seconds','').strip())
                elif line.startswith('Processes Used:'):
                    processes = int(line.split(':')[1].strip())
                elif line.startswith('Workers Used:'):
                    workers = int(line.split(':')[1].strip())
                elif line.startswith('Throughput:'):
                    throughput = float(line.split(':')[1].replace('words/sec','').strip())
                elif line.startswith('Top 20 Words:'):
                    in_top = True
                elif in_top and ':' in line:
                    parts = line.split(':')
                    word = parts[0].strip()
                    count = int(parts[1].strip())
                    top_words.append({'word': word, 'count': count})

        return jsonify({
            'success': True,
            'log': output,
            'unique_words': unique_words,
            'exec_time': exec_time,
            'processes': processes,
            'workers': workers,
            'throughput': round(throughput, 2),
            'top_words': top_words,
            'elapsed': elapsed,
            'np': np
        })

    except Exception as e:
        return jsonify({'success': False, 'error': str(e)})

if __name__ == '__main__':
    port = int(os.environ.get('PORT', 5000))
    app.run(host='0.0.0.0', port=port, debug=False)