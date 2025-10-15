% Real time data collection example
%
% This script is implemented as a function so that it can
%   include sub-functions
%
% This script can be modified to be used on any platform by changing the
% serialPort variable.
% Example:-
% On Linux:     serialPort = '/dev/ttyS0';
% On MacOS:     serialPort = '/dev/tty.KeySerial1';
% On Windows:   serialPort = 'COM1';
%
% To run: 
% plot_cameras_serial()
%
% TODO: Complete these sections
%

function plot_cameras_serial

trace = zeros(1, 128);  % Stored Values for Raw Input
plt = tiledlayout(3,1); % Plot Layout 

ax1 = nexttile;
ax2 = nexttile;
ax3 = nexttile;

try
    while (true)
        trace = readData(trace);
        smoothtrace = smoothData(trace);  % Smoothed data
        bintrace = edgeData(smoothtrace); % Edge detection

        if ~isvalid(plt), break;   end
        if isvalid(ax1), cla(ax1); end
        if isvalid(ax2), cla(ax2); end
        if isvalid(ax3), cla(ax3); end

        plotdata(trace, smoothtrace, bintrace, plt, ax1, ax2, ax3);
    end
catch
    close(plt.Parent);
end

disp('Exiting...');

end % plot_cameras_serial

%*****************************************************************************************************************
%*****************************************************************************************************************
function trace = readData(trace)
    % Initialize Serial Object
    serialPort = "COM4";
    serialBaudrate = 9600;
    camera = serialport(serialPort, serialBaudrate);
    camera.FlowControl = "software";
    camera.configureTerminator("CR/LF");
    count = 1;

    % Read data from serial object for trace
    while(true)
        disp("Searching for start..");
        val = readline(camera);
        if (strcmp(val, "-1") == 0) % if not the start
            disp(val); % words, not numbers
            continue;
        end
        disp("FOUND START!");
        while (true)
            val = str2double(readline(camera));
            disp(val);
            if (val == -2)
                break;
            else
                trace(count) = val;
                count = count + 1;
            end
        end
        break; % hit from val=-2
    end

    % Clean up the serial object
    clear camera;
end

% TODO: Complete the functions below

function data = smoothData(data)
    windowSize = 5;
    data = movmean(data, windowSize);  % Built-in moving average

end

function bintrace = edgeData(data)
    % Smooth data first to reduce noise
    smoothData = movmean(data, 5);

    % Compute derivative
    diffData = [0, diff(smoothData)];

    % Adaptive threshold based on derivative's standard deviation
    threshold = 1.25 * std(diffData);  % tweak multiplier for sensitivity

    % Identify edges
    edges = abs(diffData) > threshold;

    % Optional: keep only local maxima for cleaner edges
    for i = 2:length(edges)-1
        if edges(i) && ~(abs(diffData(i)) > abs(diffData(i-1)) && abs(diffData(i)) >= abs(diffData(i+1)))
            edges(i) = 0;
        end
    end

    bintrace = double(edges);
end

function plotdata(trace, smoothtrace, bintrace, plt, ax1, ax2, ax3)
    % TODO: Plot data
    plot(ax1, trace, 'LineWidth', 1.5); title(ax1,'Raw Trace'); ylabel(ax1,'Amplitude');
    plot(ax2, smoothtrace, 'LineWidth', 1.5); title(ax2,'Smoothed Trace'); ylabel(ax2,'Amplitude');
    plot(ax3, bintrace, 'LineWidth', 1.5); title(ax3,'Edge Detection');

    refreshdata
    drawnow
end
