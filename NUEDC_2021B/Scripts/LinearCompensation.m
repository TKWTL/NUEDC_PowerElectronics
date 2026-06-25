% MATLAB脚本用于线性补偿，处理测量值和实际值对，更新斜率和偏移，并绘制结果

% 输入原始斜率和偏移
original_slope = input('请输入原始斜率: ');
original_offset = input('请输入原始偏移: ');

% 定义实际值和测量值
x = [0,    0.991, 1.986, 2.985  , 3.989 ];%IN 实际
y = [0.08, 1.086, 2.09 , 3.103  , 4.118 ];%IN 测量
%x = [0,    0.991, 1.986, 2.985  , 3.992 ];%IN 实际
%y = [0.015, 1.04, 2.06 , 3.078  , 4.092 ];%IN 测量
%x = [41.12, 36.13 , 31.41, 26.45 , 21.41 , 0    ];%AB 实际
%y = [39.43, 34.38 , 29.70, 24.97 , 19.91 , -1.79];%AB 测量

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