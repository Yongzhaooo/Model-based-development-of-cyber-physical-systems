clf;
close all;
clear;
clc;

T = readtable('simulation.csv');

figure("Name", "Roll");
hold on
plot(T.time, T.actual_roll);
plot(T.time, T.estimated_roll);
plot(T.time, T.reference_roll);
legend("r_{true}", "r_{est}", "r_{ref}");

figure("Name", "Pitch");
hold on
plot(T.time, T.actual_pitch);
plot(T.time, T.estimated_pitch);
plot(T.time, T.reference_pitch);
legend("p_{true}", "p_{est}", "p_{ref}");

% figure("Name", "Yaw");
% hold on
% plot(T.time, T.actual_yaw);
% plot(T.time, T.estimated_yaw);
% plot(T.time, T.reference_yaw);
% legend("y_{true}", "y_{est}", "y_{ref}");
