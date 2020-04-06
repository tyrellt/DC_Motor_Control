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
mySerial = serial(port, 'BaudRate', 230400, 'FlowControl', 'hardware','Timeout',2); 
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
            fprintf('Current: %f\n',n);     % print it to the screen

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
            fprintf(mySerial, '%d %d\n', n);
            n = fscanf(mySerial,'%d');   % get status code
            if (n ~= n1 + n2) % failure code
                fprintf('Current gain set FAILED.');
            else
                fprintf('Current gains successfully set.');
            end

        case 'h'
            % Get current gains
            n = fscanf(mySerial,'%d %d');
            fprintf('Current gains: kp: %d\n', n(1));
            fprintf('               ki: %d\n', n(2));

        case 'i'
            % Set position gains
            n1 = input('Enter Kp gain: ');
            n2 = input('Enter Kd gain: ');
            n3 = input('Enter Ki gain: ');
            n = [n1; n2; n3];
            fprintf(mySerial, '%d %d %d\n', n);
            n = fscanf(mySerial,'%d');   % get status code
            if (n ~= n1 + n2 + n3) % failure code
                fprintf('Current gain set FAILED.');
            else
                fprintf('Current gains successfully set.');
            end

        case 'j'
            % Get position gains
            n = fscanf(mySerial,'%d %d %d');
            fprintf('Current gains: kp: %d\n', n(1));
            fprintf('               kd: %d\n', n(2));
            fprintf('               ki: %d\n', n(3));

        case 'k'
            % Test current control
            % read data from test and plot it

        case 'l'
            % Go to angle (deg)
            n = input('Enter desired angle (in degrees): ');
            fprintf(mySerial, '%f\n', n);
            n = fscanf(mySerial, '%f'); % just for testing. 
                                        % the pic sends back the value
            fprintf('value sent to PIC: %f\n', n);

        case 'm'
            % Load step trajectory
            traj = input('Enter step trajectory, in sec and degrees\n [time1, ang1; time2, ang2; ...]: ');
            if (traj(end,1) > 10)
                fprintf('Error: Maximum trajectory time is 10 seconds\n');
                fprintf(mySerial, '%d\n', 0);    % error code for pic
            else
                % call genRef with step option and send result to PIC
                refTraj = genRef(traj, 'step') * 100;   % refTraj stored as integer
                                                        % in hundredths of
                                                        % a degree
                                                        
                fprintf(mySerial, '%d\n', length(refTraj)); % send length of trajectory so memory can be allocated
                % fprintf(mySerial, '%d\n', refTraj);
                for i = 1:length(refTraj)
                    fprintf(mySerial, '%d\n', refTraj(i));   % this should send every entry in refTraj
                    fprintf('Point stored in PIC: %d\n', fscanf(mySerial, '%d'));
                end
            end
            
            

        case 'n'
            % Load cubic trajectory

        case 'o'
            % Execute trajectory

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
            fprintf('     x: Display Menu\n');
  
        case 'y'                         % example operation
            n = input('Enter numbers: ', 's'); % get the numbers to send
            fprintf(mySerial, '%s\n',n); % send the numbers
            %fprintf(mySerial, '%d\n', 4);
            %fprintf(mySerial, '%d\n', 5);
            n = fscanf(mySerial,'%d');   % get the sum back
            fprintf('Read: %d\n',n);     % print it to the screen
  
        otherwise
            fprintf('Invalid Selection %c\n', selection);
            endq
            
    end

end
