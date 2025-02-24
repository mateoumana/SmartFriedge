from flask import Flask

#local and web server
app = Flask(__name__)

@app.route('/') #direccion root (raiz)
def index():
    user_ip = request.remote_addr
    response = make_response(redirect('/hello'))
    response.set_cookie('user_IP', user_ip)
    return response


@app.route('/hello')
def hello():
    user_ip = request.cookies.get('user_IP')
    return 'Hello World from FLASK, tu IP es {}'.format(user_ip)
