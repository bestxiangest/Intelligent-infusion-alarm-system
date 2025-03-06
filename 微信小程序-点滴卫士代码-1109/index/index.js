// 定义页面对象，包含页面的初始数据和生命周期函数
Page({
  // 页面的初始数据
  data: {
    // 属性对象，包含液位、剩余时间、状态等信息
    properties: {
      level: 0, // 液位
      timeLeft: 0, // 剩余时间
      status: '', // 状态
      updateTime: '', // 更新时间
      predit_hour: 0, // 预计小时数
      predit_min: 0, // 预计分钟数
    },
    // 患者对象，包含患者姓名、年龄、性别、开始和结束时间
    patient: {
      name: '',
      age: '',
      gender: '',
      startTime: '', // 开始时间
      endTime: '' // 结束时间
    },
    intervalId: null, // 定时器ID
    getTimes: 0, // 获取时间的次数
  },

  // 页面加载时执行的函数
  onLoad() {
    this.startFetchingData(); // 开始获取数据
    this.startInfusion(); // 开始输液
  },

  // 页面卸载时执行的函数
  onUnload() {
    this.stopFetchingData(); // 停止获取数据
  },

  // 预留的函数，尚未实现具体功能
  yell() {

  },

  // 刷新页面数据的函数
  refresh() {
    this.getPropertiesData(); // 获取属性数据
  },

  // 开始输液的函数
  startInfusion() {
    const startTime = this.getCurrentTime().split(":")[0] + ":" + this.getCurrentTime().split(":")[1];
    this.setData({
      'patient.startTime': startTime, // 设置患者开始时间为当前时间
    });
    this.endInfusion(); // 结束输液
  },

  // 结束输液的函数
  endInfusion() {
    var endTime = this.getCurrentTime().split(" ")[0];
    var start_hour = this.data.patient.startTime.split(" ")[1].split(":")[0];
    var start_min = this.data.patient.startTime.split(" ")[1].split(":")[1];
    var predict_the_end_hour = Number(start_hour) + Number(this.data.properties.predit_hour);
    var predict_the_end_min = Number(start_min) + Number(this.data.properties.predit_min);
    console.log("predict_the_end_hour:" + predict_the_end_hour);
    console.log("predict_the_end_min:" + predict_the_end_min);
    // 处理分钟数进位
    if (predict_the_end_min >= 60) {
      predict_the_end_min = predict_the_end_min % 60;
      predict_the_end_hour += 1;
    }
    // 格式化时间显示
    if (predict_the_end_min < 10) {
      predict_the_end_min = "0" + predict_the_end_min.toString();
    }
    if (predict_the_end_hour < 10) {
      predict_the_end_hour = "0" + predict_the_end_hour.toString();
    }
    endTime += " " + predict_the_end_hour + ":" + predict_the_end_min;
    this.setData({
      'patient.endTime': endTime // 设置患者结束时间为计算后的时间
    });
  },

  // 开始获取数据的函数
  startFetchingData() {
    const that = this;
    this.getPropertiesData(); // 获取属性数据
    const intervalId = setInterval(function () {
      that.getPropertiesData(); // 每3秒获取一次数据
    }, 3000);
    this.setData({ intervalId }); // 设置定时器ID
  },

  // 停止获取数据的函数
  stopFetchingData() {
    clearInterval(this.data.intervalId); // 清除定时器
  },

  // 获取属性数据的函数
  getPropertiesData() {
    const that = this;
    wx.request({
      url: 'http://47.95.172.71:5000/getProperties', // Flask API 地址
      method: 'GET',
      success: function (res) {
        if (res.statusCode == 200) {
          console.log(res.data); // 输出来自Flask的响应数据
          // 处理 JSON 数据
          var properties = res.data.shadow[0].reported.properties;
          console.log("液位:" + properties.level);
          console.log("剩余时间:" + properties.restTime[0] + "小时" + properties.restTime[1] + "分钟");
          console.log(properties.status);
          const currentTime = that.getCurrentTime();
          that.setData({
            properties: {
              level: properties.level,
              timeLeft: properties.restTime[0] + "小时" + properties.restTime[1] + "分钟",
              status: properties.status,
              updateTime: currentTime,
              predit_hour: properties.restTime[0],
              predit_min: properties.restTime[1]
            },
          });
          // 计算结束时间
          var endTime = that.getCurrentTime().split(" ")[0];
          var start_hour = that.data.patient.startTime.split(" ")[1].split(":")[0];
          var start_min = that.data.patient.startTime.split(" ")[1].split(":")[1];
          var predict_the_end_hour = Number(start_hour) + Number(that.data.properties.predit_hour);
          var predict_the_end_min = Number(start_min) + Number(that.data.properties.predit_min);
          console.log("predict_the_end_hour:" + predict_the_end_hour);
          console.log("predict_the_end_min:" + predict_the_end_min);
          if (predict_the_end_min >= 60) {
            predict_the_end_min = predict_the_end_min % 60;
            predict_the_end_hour += 1;
          }
          if (predict_the_end_min < 10) {
            predict_the_end_min = "0" + predict_the_end_min.toString();
          }
          if (predict_the_end_hour < 10) {
            predict_the_end_hour = "0" + predict_the_end_hour.toString();
          }
          endTime += " " + predict_the_end_hour + ":" + predict_the_end_min;
          that.setData({
            'patient.endTime': endTime
          });
        } else {
          console.log("请求失败：", res.statusCode);
        }
      },
      fail: function (error) {
        console.error('请求错误:', error);
      }
    });
  },

  // 获取当前时间的函数
  getCurrentTime() {
    const now = new Date();
    const year = now.getFullYear();
    const month = (now.getMonth() + 1).toString().padStart(2, '0');
    const day = now.getDate().toString().padStart(2, '0');
    const hours = now.getHours().toString().padStart(2, '0');
    const minutes = now.getMinutes().toString().padStart(2, '0');
    const seconds = now.getSeconds().toString().padStart(2, '0');
    return `${year}-${month}-${day} ${hours}:${minutes}:${seconds}`;
  },
});
