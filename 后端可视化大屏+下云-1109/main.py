
# 导入os模块
import os
# 导入BasicCredentials和DerivedCredentials类
from huaweicloudsdkcore.auth.credentials import BasicCredentials
from huaweicloudsdkcore.auth.credentials import DerivedCredentials
# 导入Region类
from huaweicloudsdkcore.region.region import Region as coreRegion
# 导入exceptions模块
from huaweicloudsdkcore.exceptions import exceptions
# 导入IoTDAClient类
from huaweicloudsdkiotda.v5 import *
# 导入json模块
import json
# 导入Flask模块
from flask import Flask,jsonify,render_template,make_response,url_for
# 导入CORS模块
from flask_cors import CORS

# 创建Flask应用
app = Flask(__name__, static_folder="templates")
# 允许跨域访问
CORS(app)

# 定义根路由
@app.route("/",methods=['GET','POST'])
def index():
    # 返回index.html页面
    return render_template("index.html")

# 定义getProperties路由
@app.route("/getProperties",methods=['GET','POST'])
def getProperties():
    # The AK and SK used for authentication are hard-coded or stored in plaintext, which has great security risks. It is recommended that the AK and SK be stored in ciphertext in configuration files or environment variables and decrypted during use to ensure security.
    # In this example, AK and SK are stored in environment variables for authentication. Before running this example, set environment variables CLOUD_SDK_AK and CLOUD_SDK_SK in the local environment
    # 使用环境变量中的AK和SK进行认证
    ak = "TDLYQAPOMTCFM6NEEFLN"
    sk = "2PF98QYPJneXW1oRjxg4CP53OYP6ud4XUJFynHmI"
    # // ENDPOINT：请在控制台的"总览"界面的"平台接入地址"中查看“应用侧”的https接入地址。
    # 设置IoTDA的接入地址
    iotdaEndpoint = "9767762256.st1.iotda-app.cn-east-3.myhuaweicloud.com"
    # 创建BasicCredentials对象
    credentials = BasicCredentials(ak, sk).with_derived_predicate(DerivedCredentials.get_default_derived_predicate())
    # 标准版/企业版：需要使用自行创建的Region对象，基础版：请选择IoTDAClient中的Region对象 如： .with_region(IoTDARegion.CN_NORTH_4)
    # 创建IoTDAClient对象
    client = IoTDAClient.new_builder() \
        .with_credentials(credentials) \
        .with_region(coreRegion(id="cn-east-3", endpoint='9767762256.st1.iotda-app.cn-east-3.myhuaweicloud.com')) \
        .build()
    try:
        # 创建ShowDeviceShadowRequest对象
        request = ShowDeviceShadowRequest()
        # 设置设备ID
        request.device_id = "660fbe30387fa41cc8a08b04_esp32c3-002"
        # 发送请求
        response = client.show_device_shadow(request).to_str()
        # 打印响应
        print(response)
        # 将响应转换为json对象
        json_object = json.loads(response)
        # 获取properties
        json_properties = json_object["shadow"][0]["reported"]["properties"]
        # 获取level
        level = json_properties["level"]
        # 获取temp
        temp = json_properties["temp"]
        # 获取nodeStatus
        nodeStatus = json_properties["nodeStatus"]
        # 获取restTime
        restTime = json_properties["restTime"]
        # 获取startTime
        startTime = json_properties["startTime"]
        # 获取status
        status = json_properties["status"]
        # 获取tips
        tips = json_properties["tips"]

        # 打印properties
        print(str(level)+'\n'+str(temp)+'\n'+str(nodeStatus)+'\n'+str(restTime)+'\n'+str(startTime)+'\n'+str(status)+'\n'+str(tips))
        # 返回json对象
        return jsonify(json_object)

    except exceptions.ClientRequestException as e:
        # 打印异常信息
        print(e.status_code)
        print(e.request_id)
        print(e.error_code)
        print(e.error_msg)


# 主函数
if __name__ == "__main__":
    # 打印欢迎信息
    print("Welcome!")
    # 运行Flask应用
    app.run(host='127.0.0.1', port=5000,debug=False)