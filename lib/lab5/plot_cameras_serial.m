function plot_cameras_serial
    
    % --- CONFIGURATION ---
    serialPort = "COM3";    % !! CHECK YOUR PORT !!
    serialBaudrate = 9600; % !! MUST MATCH UART0 IN C CODE !!
    
    % --- SETUP PLOT ---
    f = figure('Name', 'NXP Cup Telemetry', 'Color', 'w');
    t = tiledlayout(3,1, 'Padding', 'compact');
    
    % Plot 1: Raw Data
    ax1 = nexttile;
    hRaw = plot(ax1, zeros(1, 128), '-b', 'LineWidth', 1.5);
    title(ax1, '1. Raw Camera Data');
    ylabel(ax1, 'Intensity (ADC)');
    ylim(ax1, [0 4100]);
    grid(ax1, 'on');
    
    % Plot 2: Derivative (Edges)
    ax2 = nexttile;
    hDiff = plot(ax2, zeros(1, 128), '-r', 'LineWidth', 1.5);
    yline(ax2, 0, '--k'); % Zero reference
    title(ax2, '2. Derivative (Rate of Change)');
    ylabel(ax2, 'd/dx');
    ylim(ax2, [-2000 2000]); 
    grid(ax2, 'on');

    % Plot 3: Car Decision (Line Center)
    ax3 = nexttile;
    hEdge = plot(ax3, zeros(1, 128), '-k', 'LineWidth', 2);
    hold(ax3, 'on');
    % Draw a visual marker for the calculated center
    hCenterline = xline(ax3, 64, '-g', 'LineWidth', 3, 'Label', 'CENTER');
    title(ax3, '3. Strongest Edges & Calculated Center');
    ylim(ax3, [-0.1 1.1]);
    
    % --- SERIAL CONNECTION ---
    try
        % FIX: Use serialportfind to clear only THIS specific port
        oldPorts = serialportfind("Port", serialPort);
        if ~isempty(oldPorts)
            disp("Closing existing connection to " + serialPort + "...");
            delete(oldPorts);
        end
        
        fprintf("Opening %s at %d baud...\n", serialPort, serialBaudrate);
        camera = serialport(serialPort, serialBaudrate);
        configureTerminator(camera, "CR/LF");
        
        % Auto-close port if script crashes or is stopped
        cleanupObj = onCleanup(@() delete(camera));
        
        trace = zeros(1, 128);
        
        % --- MAIN LOOP ---
        while isvalid(f)
            
            % 1. SYNC: Read until we find "-1"
            while true
                try
                    lineStr = readline(camera);
                    if str2double(lineStr) == -1
                        break; 
                    end
                catch
                    % Ignore read errors during sync
                end
            end
            
            % 2. DATA: Read 128 pixels
            for i = 1:128
                val = str2double(readline(camera));
                % Verify we didn't hit the end marker early (data corruption)
                if val == -2 
                    break; 
                end
                trace(i) = val;
            end
            
            % Consume the "-2" marker if we haven't already
            if val ~= -2
                readline(camera); 
            end

            % --- PROCESSING (Match C-Code Logic) ---
            % 1. Smooth
            smoothTrace = movmean(trace, 3);
            
            % 2. Derivative
            diffTrace = [0, diff(smoothTrace)];
            diffTrace(1) = 0; diffTrace(128) = 0;
            
            % 3. Find Strongest Edges
            [~, leftIdx] = max(diffTrace(5:123)); % Max Positive = Left Edge
            [~, rightIdx] = min(diffTrace(5:123));% Max Negative = Right Edge
            
            % Correct index offset (because we searched 5:123)
            leftIdx = leftIdx + 4;
            rightIdx = rightIdx + 4;
            
            % Calculate Center
            centerPixel = (leftIdx + rightIdx) / 2;
            
            % Binary visualization array
            binTrace = zeros(1, 128);
            binTrace(leftIdx) = 1;  % Mark Left Edge
            binTrace(rightIdx) = 1; % Mark Right Edge
            
            % --- UPDATE PLOTS ---
            set(hRaw, 'YData', trace);
            set(hDiff, 'YData', diffTrace);
            set(hEdge, 'YData', binTrace);
            set(hCenterline, 'Value', centerPixel); 
            
            % Update title with error value
            errVal = (centerPixel - 64) / 64;
            title(ax3, sprintf('Center: %.1f | Error: %.2f', centerPixel, errVal));

            drawnow limitrate; % Efficient rendering
        end
        
    catch ME
        fprintf("Error: %s\n", ME.message);
    end
end