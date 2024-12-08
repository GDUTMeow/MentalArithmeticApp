from flask import Flask, render_template, send_from_directory

app = Flask(__name__)

app.template_folder = "templates"

@app.route('/static/<path:filename>')
def serve_static(filename):
    return send_from_directory('static', filename)

@app.route("/", defaults={"path": ""})
@app.route("/<path:path>")
def render_templates_from_url(path: str):
    return render_template(
        path,
        exam = {
            "metadata": {
                "title": "这是一场不存在的考试",
                "description": "Lorem ipsum dolor sit amet, Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat."
            }
        }
        )

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=1)