% generate_test_vectors.m
% Generate reference test vectors from MATLAB for validation

output_dir = 'test_vectors';
if ~exist(output_dir, 'dir')
    mkdir(output_dir);
end

%% FFT Tests
N = 256;
x = sin(2*pi*(0:N-1)/N * 8) + 0.5*cos(2*pi*(0:N-1)/N * 16);
X = fft(x);

fft_test.input = x;
fft_test.output_real = real(X);
fft_test.output_imag = imag(X);
save(fullfile(output_dir, 'fft_test.mat'), '-struct', 'fft_test');

%% Window Functions
window_test.hann = hann(64)';
window_test.hamming = hamming(64)';
window_test.blackman = blackman(64)';
save(fullfile(output_dir, 'window_test.mat'), '-struct', 'window_test');

%% Butterworth Filter
[b_butter, a_butter] = butter(4, 0.2, 'low');
butter_test.b = b_butter;
butter_test.a = a_butter;
butter_test.order = 4;
butter_test.normalized_freq = 0.2;
save(fullfile(output_dir, 'butter_test.mat'), '-struct', 'butter_test');

%% Filter Application
x_filter = randn(1, 200);
y_filter = filter(b_butter, a_butter, x_filter);
y_filtfilt = filtfilt(b_butter, a_butter, x_filter);

filter_test.input = x_filter;
filter_test.b = b_butter;
filter_test.a = a_butter;
filter_test.output_filter = y_filter;
filter_test.output_filtfilt = y_filtfilt;
save(fullfile(output_dir, 'filter_test.mat'), '-struct', 'filter_test');

%% Cross-Correlation
x_corr = [1 2 3 4 5];
y_corr = [1 2 3 4 5];
r_corr = xcorr(x_corr, y_corr);

corr_test.x = x_corr;
corr_test.y = y_corr;
corr_test.output = r_corr;
save(fullfile(output_dir, 'correlation_test.mat'), '-struct', 'corr_test');

%% Hilbert Transform
x_hilbert = sin(2*pi*(0:99)/100 * 5);
z_hilbert = hilbert(x_hilbert);

hilbert_test.input = x_hilbert;
hilbert_test.output_real = real(z_hilbert);
hilbert_test.output_imag = imag(z_hilbert);
hilbert_test.envelope = abs(z_hilbert);
save(fullfile(output_dir, 'hilbert_test.mat'), '-struct', 'hilbert_test');

%% Resample
x_resample = sin(2*pi*(0:99)/100 * 3);
y_resample = resample(x_resample, 3, 2);

resample_test.input = x_resample;
resample_test.p = 3;
resample_test.q = 2;
resample_test.output = y_resample;
save(fullfile(output_dir, 'resample_test.mat'), '-struct', 'resample_test');

%% Welch PSD
x_psd = randn(1, 1000);
[Pxx, F] = pwelch(x_psd, hann(256), 128, 256, 1000);

psd_test.input = x_psd;
psd_test.psd = Pxx';
psd_test.frequencies = F';
psd_test.segment_size = 256;
psd_test.overlap = 128;
psd_test.sample_rate = 1000;
save(fullfile(output_dir, 'psd_test.mat'), '-struct', 'psd_test');

fprintf('Test vectors generated in %s\n', output_dir);
