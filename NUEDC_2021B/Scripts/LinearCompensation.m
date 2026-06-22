% MATLAB脚本用于线性补偿，处理测量值和实际值对，更新斜率和偏移，并绘制结果

% 输入原始斜率和偏移
original_slope = input('请输入原始斜率: ');
original_offset = input('请输入原始偏移: ');

% 定义实际值和测量值
% x = [23.80, 17.00 , 8.060, 0.0318  , -8.057 ,-16.600 ,-24.65  ];%UV 实际
% y = [22.30, 15.05 , 5.49 , -3.11   , -11.75 ,-20.83  ,-29.34  ];%UV 测量
x = [24.26, 16.60 , 8.406, 0.0247  , -8.395 , -16.130 ,-24.22  ];%WV 实际
y = [22.92, 14.70 , 5.89 , -3.07   , -12.06 , -20.87 ,-28.87  ];%WV 测量
%y = [0.473, 0.760, 0.961, 1.448, 1.936];
%x = [0.5, 0.8, 1, 1.5, 2];

% 检查是否至少有两个数据对
if length(x) < 2
    disp('需要至少两个数据对进行拟合。');
    return;
end

% 计算i
i = (y - original_offset) ./ original_slope;

% 线性回归：x = m*i + b
p = polyfit(i, x, 1);
m = p(1);
b = p(2);

% 输出更新后的斜率和偏移
disp(['更新后的斜率: ', num2str(m)]);
disp(['更新后的偏移: ', num2str(b)]);

% 绘制图形
x_range = linspace(min(x), max(x), 100);
figure;
hold on;
scatter(x, y, 'o');
plot(x_range, x_range, 'k--');
plot(x_range, original_slope * x_range + original_offset, 'r');
plot(x_range, m * x_range + b, 'g');
hold off;
title('校准图');
xlabel('实际值');
ylabel('测量值');
legend('数据点', '实际值线', '原始线', '补偿线');