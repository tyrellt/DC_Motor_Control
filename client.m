function client()
%   provides a menu for accessing PIC32 motor control functions
%
%   client(port)
%
%   Input Arguments:
%       port - the name of the com port.  This should be the same as what
%               you use in screen or putty in quotes ' '
%
%   Example:
%       client('/dev/ttyUSB0') (Linux/Mac)
%       client('COM3') (PC)
%
%   For convenience, you may want to change this so that the port is hardcoded.

% constants
MAX_TRAJ_POSITIONS = 20;    % if this is changed, it needs to be changed in main.c as well

% Opening COM connection
if ~isempty(instrfind)
    fclose(instrfind);
    delete(instrfind);
end

port = 'COM6';

fprintf('Opening port %s....\n',port);

% settings for opening the serial port. baud rate 230400, hardware flow control
% wait up to 120 seconds for data before timing out
mySerial = serial(port, 'BaudRate', 230400, 'FlowControl', 'hardware','Timeout',15); 
% opens serial connection
fopen(mySerial);
% closes serial port when function exits
clean = onCleanup(@()fclose(mySerial));                                 

has_quit = false;
% menu loop
fprintf('PIC32 MOTOR DRIVER INTERFACE\n\n');
fprintf('     x: Display Menu\n');
while ~has_quit

    % read the user's choice
    selection = input('\nENTER COMMAND: ', 's');
     
    % send the command to the PIC32
    if (selection ~= 'x')
        fprintf(mySerial,'%c\n',selection);
    end
    % take the appropriate action
    switch selection
        case 'a'
            % Read current sensor (ADC counts)
            n = fscanf(mySerial,'%d');   % get ADC counts back
            fprintf('ADC Counts: %d\n',n);     % print it to the screen

        case 'b'
            % Read current sensor (mA)
            n = fscanf(mySerial,'%f');   % get current reading back
            fprintf('Current: %.2f mA\n',n);     % print it to the screen

        case 'c'
            % Read encoder (counts)
            n = fscanf(mySerial,'%d');
            fprintf('Encoder Counts: %d\n',n);

        case 'd'
            % Read encoder (deg)
            n = fscanf(mySerial,'%f');
            fprintf('Position (deg): %f\n',n);

        case 'e'
            % Reset encoder
            % read encoder counts to confirm reset
            n = fscanf(mySerial,'%d');
            fprintf('Counts reset. Encoder Counts: %d\n',n);

        case 'f'
            % Set PWM
            n = input('Enter PWM duty cycle (-100 to 100): ');
            fprintf(mySerial, '%d\n',n);
            n = fscanf(mySerial,'%d');   % get status code
            if (n == 0) % failure code
                fprintf('Duty cycle update FAILED.');
            else
                fprintf('Duty cycle successfully updated.');
            end
                
        case 'g'
            % Set current gains
            n1 = input('Enter Kp gain: ');
            n2 = input('Enter Ki gain: ');
            n = [n1; n2];
            fprintf(mySerial, '%f %f\n', n);
            n = fscanf(mySerial,'%f');   % get sum
            fprintf('Current gains successfully set.');

        case 'h'
            % Get current gains
            n = fscanf(mySerial,'%f %f');
            fprintf('Current gains: kp: %f\n', n(1));
            fprintf('               ki: %f\n', n(2));

        case 'i'
            % Set position gains
            n1 = input('Enter Kp gain: ');
            n2 = input('Enter Kd gain: ');
            n3 = input('Enter Ki gain: ');
            n = [n1; n2; n3];
            fprintf(mySerial, '%f %f %f\n', n);
            n = fscanf(mySerial,'%f');   % get sum
            fprintf('Position gains successfully set.');

        case 'j'
            % Get position gains
            n = fscanf(mySerial,'%f %f %f');
            fprintf('Position gains: kp: %f\n', n(1));
            fprintf('                kd: %f\n', n(2));
            fprintf('                ki: %f\n', n(3));

        case 'k'
            % Test current control
            itestPlot(mySerial);
            
        case 'l'
            % Go to angle (deg)
            n = input('Enter desired angle (in degrees): ');
            fprintf(mySerial, '%f\n', n);
            n = fscanf(mySerial, '%f'); % just for testing. 
                                        % the pic sends back the value
            fprintf('value sent to PIC: %f\n', n);

        case 'm'
            % Load step trajectory
            loadTrajectory(mySerial, 'step');

        case 'n'
            % Load cubic trajectory
            loadTrajectory(mySerial, 'cubic');

        case 'o'
            % Execute trajectory
            trajPlot(mySerial);

        case 'p'
            % Unpower the motor
      
        case 'q'
            has_quit = true;             % exit client
            
        case 'r'
            % Get mode
            n = fscanf(mySerial,'%d');
            switch n
                case 0
                    mode = 'IDLE';
                case 1
                    mode = 'PWM';
                case 2
                    mode = 'ITEST';
                case 3
                    mode = 'HOLD';
                otherwise  % case 4
                    mode = 'TRACK';
            end
            fprintf('mode: %s\n', mode);
            
        case 's'
            % Sample current sensor for use with filter calculations
            numSamples = input('Enter number of samples: ');
            fprintf(mySerial, '%d\n', numSamples);
            currentSamples = [];
            ref = [];
            for i = 1:numSamples
                 n = fscanf(mySerial,'%f %f');
                 currentSamples = [currentSamples n(1)];
                 ref = [ref n(2)];
            end
            
            %plotFFT(currentSamples);
            close all;
            figure(1);
            hold on;
            stairs(currentSamples);
            leng = length(currentSamples);
            stairs(ref);
            ylabel('current (mA)');
            xlabel('sample number');
            legend('filtered','raw');
            hold off;
            
            
        case 'x'    % display menu
            fprintf('PIC32 MOTOR DRIVER INTERFACE\n\n');
            % display the menu options; this list will grow
            fprintf('     a: Read Current (ADC counts)  b: Read Current (mA)\n');
            fprintf('     c: Read Encoder (counts)      d: Read Encoder (deg)\n');
            fprintf('     e: Reset encoder              f: Set PWM (-100 to 100)\n');
            fprintf('     g: Set current gains          h: Get current gains\n');
            fprintf('     i: Set position gains         j: Get position gains\n');
            fprintf('     k: Test current control       l: Go to angle (deg)\n');
            fprintf('     m: Load step trajectory       n: Load cubic trajectory\n');
            fprintf('     o: Execute trajectory         p: Unpower the motor\n');
            fprintf('     q: Quit client                r: Get mode\n');
            fprintf('     s: Sample for filter          x: Display Menu\n');
  
        case 'y'                         % example operation
            n = input('Enter numbers: ', 's'); % get the numbers to send
            fprintf(mySerial, '%s\n',n); % send the numbers
            %fprintf(mySerial, '%d\n', 4);
            %fprintf(mySerial, '%d\n', 5);
            n = fscanf(mySerial,'%d');   % get the sum back
            fprintf('Read: %d\n',n);     % print it to the screen
  
        otherwise
            fprintf('Invalid Selection %c\n', selection);
            
    end

end
end

function loadTrajectory(serial, type)
    traj = input(['Enter ' type ' trajectory, in sec and degrees\n [time1, ang1; time2, ang2; ...]: ']);
    if (traj(end,1) > 10)
        fprintf('Error: Maximum trajectory time is 10 seconds\n');
        fprintf(serial, '%d\n', 0);    % error code for pic
    else
        % call genRef with step option and send result to PIC
        refTraj = genRef(traj, type);   % refTraj stored as float

        fprintf(serial, '%d\n', length(refTraj)); % send length
        for i = 1:length(refTraj)
            fprintf(serial, '%f\n', refTraj(i));
            %fprintf('Point stored in PIC: %f\n', fscanf(serial, '%f'));
        end
    end
end


