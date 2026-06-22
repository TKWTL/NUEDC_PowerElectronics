%% 1. 读取 CSV 文件（从第 20 行开始）
filename = 'WAVE.csv';  % 替换为你的CSV文件名
opts = detectImportOptions(filename);
opts.DataLines = [20 Inf];   % 从第20行开始读
data = readmatrix(filename, opts);

% 提取时间和电压数据
t = data(:,1);        % 时间列1（秒）
v1 = data(:,2);       % 通道1电压（V）
t2 = data(:,3);       % 时间列2（应与t基本一致）
v2 = data(:,4);       % 通道2电压（V）

% 检查时间戳一致性
assert(max(abs(t - t2)) < 1e-6, '两通道时间戳不一致，需同步');

%% 2. 采样率计算（共用）
Fs = 1 / mean(diff(t));  % 采样频率
fprintf('Sampling Rate: %.2f Hz\n', Fs);

%% 3. 分别计算每个通道的 RMS 和 THD
% 通道 1
Vrms1 = sqrt(mean(v1.^2));
thd_dB1 = thd(v1, Fs);
thd_percent1 = 100 * 10^(thd_dB1 / 20);

% 通道 2
Vrms2 = sqrt(mean(v2.^2));
thd_dB2 = thd(v2, Fs);
thd_percent2 = 100 * 10^(thd_dB2 / 20);

% 输出结果
fprintf('=== 通道 1 (CH1) ===\n');
fprintf('RMS  = %.4f V\n', Vrms1);
fprintf('THD  = %.2f dB ( %.4f %% )\n\n', thd_dB1, thd_percent1);

fprintf('=== 通道 2 (CH2) ===\n');
fprintf('RMS  = %.4f V\n', Vrms2);
fprintf('THD  = %.2f dB ( %.4f %% )\n', thd_dB2, thd_percent2);

%% 4. 波形可视化（时间域）—— 仅显示两个原始通道
figure('Name', 'Time-Domain Waveforms');
plot(t, v1, 'b', 'LineWidth', 1.2);
hold on;
plot(t, v2, 'r', 'LineWidth', 1.2);
hold off;
legend('Channel 1 (CH1)', 'Channel 2 (CH2)');
xlabel('Time (s)');
ylabel('Voltage (V)');
title('Time-Domain Waveforms - CH1 & CH2');
grid on;

%% 5. 频谱可视化（频域）—— 两个通道分别绘图或子图显示
n = length(t);
f = (0:n-1)*(Fs/n);                     % 频率轴
f_plot = f(1:floor(n/2));               % 单边谱频率
f_max = 200e3;                          % 限制最大显示频率 200 kHz
idx_max = find(f_plot <= f_max, 1, 'last');

% 计算单边谱（幅值 ×2，除直流分量）
V_fft1 = abs(fft(v1)) / n;
V_plot1 = 2 * V_fft1(1:floor(n/2));
V_fft2 = abs(fft(v2)) / n;
V_plot2 = 2 * V_fft2(1:floor(n/2));

% 转换为 dB（避免 log(0)，设置底噪 -120 dB）
V_plot1_dB = 20*log10(V_plot1);
V_plot1_dB(V_plot1_dB < -120) = -120;
V_plot2_dB = 20*log10(V_plot2);
V_plot2_dB(V_plot2_dB < -120) = -120;

% 绘图：使用子图并排显示两个通道的频谱
figure('Name', 'Frequency Spectrum (Up to 200 kHz)');
subplot(1,2,1);
semilogy(f_plot(1:idx_max), V_plot1(1:idx_max), 'b');
title('CH1 Spectrum');
xlabel('Frequency (Hz)');
ylabel('Amplitude (V)');
xlim([0 f_max]);
grid on;

subplot(1,2,2);
semilogy(f_plot(1:idx_max), V_plot2(1:idx_max), 'r');
title('CH2 Spectrum');
xlabel('Frequency (Hz)');
ylabel('Amplitude (V)');
xlim([0 f_max]);
grid on;

sgtitle('Frequency Spectrum (Log Scale, Limited to 200 kHz)');