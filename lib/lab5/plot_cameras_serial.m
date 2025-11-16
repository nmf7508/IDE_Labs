% Real-time data collection from MSP camera
%
% This script opens a serial port and continuously reads and plots
% 128-pixel frames from the camera, which are delimited by
% "-1" (start) and "-2" (end) markers.
%
% To run:
% plot_cameras_serial()
%
function plot_cameras_serial
    
    % --- Serial Port Configuration ---
    serialPort = "COM4"; % !! Make sure this is your correct COM port
    serialBaudrate = 9600; % !! MUST match your C code's UART0 baud rate
    
    trace = zeros(1, 128);  % Stored Values for Raw Input
    plt = tiledlayout(3,1); % Plot Layout 
    
    % Check if layout is still valid (user might have closed window)
    if ~isvalid(plt)
        disp('Plot window was closed.');
        return;
    end
    
    ax1 = nexttile;
    ax2 = nexttile;
    ax3 = nexttile;
    
    try
        % --- Open Serial Port ONCE ---
        disp("Opening serial port " + serialPort + "...");
        camera = serialport(serialPort, serialBaudrate);
        camera.configureTerminator("CR/LF"); % Matches C code's "\r\n"
        
        % This is a "cleanup" object. It automatically runs 'clear(camera)'
        % when the function exits for any reason (error, Ctrl-C, etc.)
        cleanupObj = onCleanup(@() clear(camera));
        
        disp("Port open. Starting data stream...");

        while (true)
            % Pass the open camera object to the read function
            trace = readData(camera, trace);
            
            smoothtrace = smoothData(trace);  % Smoothed data
            bintrace = edgeData(smoothtrace); % Edge detection
            
            % Check if user closed the plot window
            if ~isvalid(plt), break; end
            
            % Plot the data
            plotdata(trace, smoothtrace, bintrace, plt, ax1, ax2, ax3);
        end
        
    catch ME % Catch errors (e.g., port disconnect)
        disp('Error or plot closed. Exiting...');
        disp(ME.message); % Display the error
    end
    
    disp('Script finished.');

end % plot_cameras_serial

%*****************************************************************************************************************
% readData: Reads one full 128-pixel frame from the serial port
%*****************************************************************************************************************
function trace = readData(camera, trace)
    % This function now receives the already-opened 'camera' object

    % 1. Search for the start-of-frame marker "-1"
    while(true)
        val = readline(camera);
        
        % --- FIX 2 (strcmp logic) ---
        % Keep reading lines UNTIL we find "-1"
        % strcmp returns 0 if equal, so we check for not-equal
        if (strcmp(val, "-1") ~= 0) 
            % disp("Got: " + val); % Uncomment for deep debugging
            continue; % Not the start, keep searching
        end
        
        % If we are here, we found "-1"
        % disp("FOUND START!"); % Uncomment for debugging
        
        % --- FIX 3 (count reset) ---
        % Reset pixel counter to 1 for this new frame
        count = 1; 
        
        % 2. Read pixel data until end-of-frame marker "-2"
        while (true)
            valStr = readline(camera);
            val = str2double(valStr);
            
            if (val == -2)
                % Found end marker, frame is complete
                % disp("FOUND END!"); % Uncomment for debugging
                break; % Break from *inner* loop
            else
                % This is a valid pixel number
                if (count <= 128) % Safety check
                    trace(count) = val;
                end
                count = count + 1;
            end
        end
        
        break; % Break from *outer* loop (we have our frame)
    end
    
    % The function returns the completed 'trace'
end

%*****************************************************************************************************************
% smoothData: Applies a moving average filter
%*****************************************************************************************************************
function data = smoothData(data)
    windowSize = 5;
    data = movmean(data, windowSize);  % Built-in moving average
end

%*****************************************************************************************************************
% edgeData: Detects edges using a derivative and adaptive threshold
%*****************************************************************************************************************
function bintrace = edgeData(data)
    % Smooth data first to reduce noise
    smoothData = movmean(data, 5);
    
    % Compute derivative (pixel-to-pixel change)
    diffData = [0, diff(smoothData)];
    
    % Adaptive threshold based on derivative's standard deviation
    threshold = 1.25 * std(diffData);  % Tweak multiplier for sensitivity
    
    % Identify edges where the change is greater than the threshold
    edges = abs(diffData) > threshold;
    
    % Optional: Non-maximal suppression for cleaner edges
    % (Keeps only the peak of the derivative)
    for i = 2:length(edges)-1
        if edges(i) && ~(abs(diffData(i)) > abs(diffData(i-1)) && abs(diffData(i)) >= abs(diffData(i+1)))
            edges(i) = 0;
        end
    end
    
    bintrace = double(edges);
end

%*****************************************************************************************************************
% plotdata: Updates the three plots
%*****************************************************************************************************************
function plotdata(trace, smoothtrace, bintrace, plt, ax1, ax2, ax3)
    
    % Clear axes before plotting new data
    cla(ax1); 
    cla(ax2);
    cla(ax3);
    
    % Plot new data
    plot(ax1, trace, 'LineWidth', 1.5, 'Color', 'b'); 
    title(ax1,'Raw Camera Trace'); 
    ylabel(ax1,'ADC Value');
    xlim(ax1, [1 128]);
    
    plot(ax2, smoothtrace, 'LineWidth', 1.5, 'Color', 'r'); 
    title(ax2,'Smoothed Trace (Moving Average)'); 
    ylabel(ax2,'Smoothed Value');
    xlim(ax2, [1 128]);
    
    plot(ax3, bintrace, 'LineWidth', 1.5, 'Color', 'k'); 
    title(ax3,'Edge Detection (Derivative)');
    xlabel(ax3, 'Pixel Number');
    xlim(ax3, [1 128]);
    ylim(ax3, [-0.1 1.1]); % Set Y-axis for binary 0/1 data
    
    % Force MATLAB to update the plot window immediately
    drawnow
end