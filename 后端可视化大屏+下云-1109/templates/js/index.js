// 立即执行函数，防止变量污染 (function() {})();

// 柱状图模块1
(function () {
  // 1.实例化对象
  var myChart = echarts.init(document.querySelector(".bar .chart"));
  // 2.指定配置项和数据
  var option = {
    color: ['#2f89cf'],
    // 提示框组件
    tooltip: {
      trigger: 'axis',
      axisPointer: { // 坐标轴指示器，坐标轴触发有效
        type: 'shadow' // 默认为直线，可选为：'line' | 'shadow'
      }
    },
    // 修改图表位置大小
    grid: {
      left: '0%',
      top: '10px',
      right: '0%',
      bottom: '4%',
      containLabel: true
    },
    // x轴相关配置
    xAxis: [{
      type: 'category',
      data: ["18岁以下", "18-45岁", "45-60岁", "60-74岁", "74-90岁", "90岁以上"],
      axisTick: {
        alignWithLabel: true
      },
      // 修改刻度标签，相关样式
      axisLabel: {
        color: "rgba(255,255,255,0.8)",
        fontSize: 10
      },
      // x轴样式不显示
      axisLine: {
        show: false
      }
    }],
    // y轴相关配置
    yAxis: [{
      type: 'value',
      // 修改刻度标签，相关样式
      axisLabel: {
        color: "rgba(255,255,255,0.6)",
        fontSize: 12
      },
      // y轴样式修改
      axisLine: {
        lineStyle: {
          color: "rgba(255,255,255,0.6)",
          width: 2
        }
      },
      // y轴分割线的颜色
      splitLine: {
        lineStyle: {
          color: "rgba(255,255,255,0.1)"
        }
      }
    }],
    // 系列列表配置
    series: [{
      name: '人数',
      type: 'bar',
      showBackground: true,
      barWidth: '35%',
      // ajax传动态数据
      data: [parseInt (Math.random()*100), parseInt (Math.random()*100), parseInt (Math.random()*100), parseInt (Math.random()*100), parseInt (Math.random()*100), parseInt (Math.random()*100)],
      label: {
        show: true,
        // 修改标签样式
        position: 'top',
       distance:5,
       verticalAlign: 'middle',
       textStyle: {
        color: "rgba(255,255,255,0.8)",
        fontSize: 14
      }
    },
      itemStyle: {
        // 修改柱子圆角
        barBorderRadius: 5,
        
      }
    }]
  };
  // 3.把配置项给实例对象
  myChart.setOption(option);

  // 4.让图表随屏幕自适应
  window.addEventListener("resize", function () {
    myChart.resize();
  })
})();

// 柱状图模块2
(function () {
  // 1.实例化对象
  var myChart = echarts.init(document.querySelector(".bar2 .chart"));

  // 声明颜色数组
  var myColor = ["#1089E7", "#F57474", "#56D0E3", "#F8B448", "#8B78F6"];
  // 2.指定配置项和数据
  
  var option = {
    grid: {
      top: "10%",
      left: '22%',
      bottom: '10%',
      // containLabel: true
    },
    xAxis: {
      // 不显示x轴相关信息
      show: false
    },
    yAxis: [{
      type: 'category',
      // y轴数据反转，与数组的顺序一致
      inverse: true,
      // 不显示y轴线和刻度
      axisLine: {
        show: false
      },
      axisTick: {
        show: false
      },
      // 将刻度标签文字设置为白色
      axisLabel: {
        color: "#fff"
      },
      data: ["氨苄西林", "美洛西林", "头孢唑林", "头孢氨苄", "乙酰氨基酚"]
    }, {
      // y轴数据反转，与数组的顺序一致
      inverse: true,
      show: true,
      // 不显示y轴线和刻度
      axisLine: {
        show: false
      },
      axisTick: {
        show: false
      },
      // 将刻度标签文字设置为白色
      axisLabel: {
        color: "#fff"
      },
      data: [parseInt (Math.random()*100), parseInt (Math.random()*100), parseInt (Math.random()*100), parseInt (Math.random()*100), parseInt (Math.random()*100)]
    }],
    series: [{
        // 第一组柱子（条状）
        name: '条',
        type: 'bar',
        // 柱子之间的距离
        barCategoryGap: 50,
        // 柱子的宽度
        barWidth: 10,
        // 层级 相当于z-index
        yAxisIndex: 0,
        // 柱子更改样式
        itemStyle: {
          barBorderRadius: 20,
          // 此时的color可以修改柱子的颜色
          color: function (params) {
            // params 传进来的是柱子的对象
            // dataIndex 是当前柱子的索引号
            // console.log(params);
            return myColor[params.dataIndex];
          }
        },
        data: [parseInt (Math.random()*100), parseInt (Math.random()*100), parseInt (Math.random()*100), parseInt (Math.random()*100), parseInt (Math.random()*100)],
        // 显示柱子内的百分比文字
        label: {
          show: true,
          position: "inside",
          // {c} 会自动解析为数据（data内的数据）
          formatter: "{c}%"
        }
      },
      {
        // 第二组柱子（框状 border）
        name: '框',
        type: 'bar',
        // 柱子之间的距离
        barCategoryGap: 50,
        // 柱子的宽度
        barWidth: 14,
        // 层级 相当于z-index
        yAxisIndex: 1,
        // 柱子修改样式
        itemStyle: {
          color: "none",
          borderColor: "#00c1de",
          borderWidth: 2,
          barBorderRadius: 15,
        },
        data: [100, 100, 100, 100, 100]
      }
    ]
  };
  // 3.把配置项给实例对象
  myChart.setOption(option);

  // 4.让图表随屏幕自适应
  window.addEventListener('resize', function () {
    myChart.resize();
  })
})();

// 折线图模块1
(function () {
  // 年份对应数据
  var yearData = [{
      year: "2022", // 年份
      data: [
        // 两个数组是因为有两条线
        [24, 40, 101, 134, 90, 230, 210, 230, 120, 230, 210, 120],
        [40, 64, 191, 324, 290, 330, 310, 213, 180, 200, 180, 79]
      ]
    },
    {
      year: "2023", // 年份
      data: [
        // 两个数组是因为有两条线
        [123, 175, 112, 197, 121, 67, 98, 21, 43, 64, 76, 38],
        [143, 131, 165, 123, 178, 21, 82, 64, 43, 60, 19, 34]
      ]
    }
  ];
  // 4.让图表随屏幕自适应
  window.addEventListener("resize", function () {
    myChart.resize();
  })
  // 5.点击切换2022 和 2023 的数据
  $('.line h2 a').on('click', function () {
    // console.log($(this).index());
    // 点击a 之后 根据当前a的索引号 找到对应的 yearData 相关对象
    // console.log(yearData[$(this).index()]);
    var obj = yearData[$(this).index()];
    option.series[0].data = obj.data[0];
    option.series[1].data = obj.data[1];
    // 选中年份高亮
    $('.line h2 a').removeClass('a-active');
    $(this).addClass('a-active');

    // 需要重新渲染
    myChart.setOption(option);
  })
})();
// 饼形图1
(function () {
  var myChart = echarts.init(document.querySelector(".pie .chart"));
  var option;

  function makeRandomData() {
return [
{
  value: parseInt (Math.random()*100) ,
  name: "男"
},
{
  value: parseInt (Math.random()*100),
  name: "女"
}
];
}
  var option = {
    color: ["#1089E7", "#F57474", "#56D0E3", "#F8B448", "#8B78F6","#FF0000"],
    tooltip: {
      trigger: 'item',
      formatter: '{a} <br/>{b}: {c} ({d}%)'
    },
    legend: {
      // 垂直居中,默认水平居中
      // orient: 'vertical',

      bottom: 0,
      left: 10,
      // 小图标的宽度和高度
      itemWidth: 10,
      itemHeight: 10,
      // 修改图例组件的文字为 12px
      textStyle: {
        color: "rgba(255,255,255,.5)",
        fontSize: "10"
      }
    },
    series: [{
      name: '患者男女比例',
      type: 'pie',
      // 设置饼形图在容器中的位置
      center: ["50%", "42%"],
      // 修改饼形图大小，第一个为内圆半径，第二个为外圆半径
      radius: ['40%', '60%'],
      avoidLabelOverlap: false,
      // 图形上的文字
      // label: {
      //   show: false,
      //   position: 'center'
      // },
      // 链接文字和图形的线
      // labelLine: {
      //   show: false
      // },
      label: {
        fontsize: 10
      },
      // 引导线调整
      labelLine: {
        // 连接扇形图线长(斜线)    
        length: 6,
        // 连接文字线长(横线)
        length2: 8
      },
      data: makeRandomData()
    }]
  };

  setInterval(() => {
    myChart.setOption({
      series: {
        data: makeRandomData()
      }
    });
  }, 5000);
  myChart.setOption(option);
  
  window.addEventListener("resize", function () {
    myChart.resize();
  })
})();

// 饼形图2
(function () {
  var myChart = echarts.init(document.querySelector('.pie2 .chart1'));
  var option;
  function makeRandomData() {
    return [
      {
        value: parseInt ( ((Math.random()+5)*10) + ((Math.random()+2)*10)),
        name: "100%"
      },
      {
        value: parseInt ( ((Math.random()+5)*10) + ((Math.random()+2)*10)),
        name: "80%"
      },
      {
        value: parseInt ( ((Math.random()+5)*10) + ((Math.random()+2)*10)),
        name: "60%"
      },
      {
        value: parseInt ( ((Math.random()+5)*10) + ((Math.random()+2)*10)),
        name: "40%"
      },
      {
        value: parseInt ( ((Math.random()+5)*10) + ((Math.random()+2)*10)),
        name: "20%"
      },
      {
        value: parseInt ( ((Math.random()+5)*10) + ((Math.random()+2)*10)),
        name: "10%以下"
      },
      {
        value: parseInt ( ((Math.random()+5)*10) + ((Math.random()+2)*10)),
        name: "5%以下"
      }
    ];
  }
  var option = {
    color: ['#60cda0', '#ed8884', '#9fe6b8', '#0096ff', '#ff9f7f', '#FF409f', '#FF0000'],
    tooltip: {
      trigger: 'item',
      formatter: '{a} <br/>{b} : {c} ({d}%)'
    },
    legend: {
      bottom: -5,
      itemWidth: 10,
      itemHeight: 10,
      textStyle: {
        color: "rgba(255,255,255,.5)",
        fontSize: 10
      }
    },
    series: [{
      name: '剩余输液量',
      type: 'pie',
      radius: ["10%", "75%"],
      center: ['50%', '45%'],
      // 半径模式  area面积模式
      roseType: 'radius',
      // 图形的文字标签
      label: {
        fontsize: 10
      },
      // 引导线调整
      labelLine: {
        // 连接扇形图线长(斜线)
        length: 6,
        // 连接文字线长(横线)
        length2: 8
      },
      data: makeRandomData()
      // data: [{
        
      //   value: parseInt (Math.random()*100),
      //   name: "100%"
      // },
      // {
      //   value: parseInt (Math.random()*100),
      //   name: "80%"
      // },
      // {
      //   value: parseInt (Math.random()*100),
      //   name: "60%"
      // },
      // {
      //   value: parseInt (Math.random()*100),
      //   name: "40%"
      // },
      // {
      //   value: parseInt (Math.random()*100),
      //   name: "20%"
      // },
      // {
      //   value: parseInt (Math.random()*100),
      //   name: "10%以下"
      // },
      // {
      //   value: parseInt (Math.random()*100),
      //   name: "5%以下"
      // }
      // ]
    }]
  };

  setInterval(() => {
    myChart.setOption({
      series: {
        data: makeRandomData()
      }
    });
  }, 5000);
  myChart.setOption(option);
  
  window.addEventListener("resize", function () {
    myChart.resize();
  })
})();
// 每隔5秒刷新页面
// setInterval(function(){
//   location.reload();
// }, 5000);

// setInterval(function() {
//   var currentPower = parseInt(document.querySelector('.sqzs h100 h1 span').innerHTML);
//   var randomIncrement = Math.floor(Math.random() * 11); // 生成一个0到10之间的随机增量
//   var newPower = currentPower + randomIncrement;
//   document.querySelector('.sqzs h100 h1 span').innerHTML = newPower;
// }, 5000); // 每隔5秒钟更新一次数据
