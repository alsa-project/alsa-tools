
window_size = 2048;
f = 0:48000/window_size:48000 * (1 - 1/window_size);
w = transpose(hamming(1536));
n = size(foo,1);

spectrum = zeros(1,window_size);

for i = [1:n] 
	data = w .* foo(i,:);	
	spectrum = spectrum + abs(fft(data,window_size));
end

plot(f,10*log10(spectrum/max(spectrum)));
grid;
axis([0 24000 -40 0]);

