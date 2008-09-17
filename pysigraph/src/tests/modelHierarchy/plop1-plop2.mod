# tree 1.0

*BEGIN TREE top_adaptive
nb_learn_data      0
significant_labels plop1-plop2 unknown
void_label         unknown
weight             3

*BEGIN TREE ad_leaf
work          mlp1
learn_state   0
nb_learn_data 0

*BEGIN TREE sub_ad_mlp
eta                       0.15
mean                      0.952381 0.0952381 -4.005 -2.36271 0.670619 -4.57076 -2.29933 0.523571 -41.9445 -0.538333 -3.77429 0.952381 0.829219 -0.158758 0.310832 -0.0170135 0.0726155 0.0531086 1406.7 36.3724 7.94095 1 1 0 1.42857 9.28571 4.09319
name                      mlp1
net                       plop1-plop2.net
nstats                    21
sigma                     0.212959 0.293544 12.6092 7.38448 3.78191 14.4333 7.12266 4.00781 9.47667 2.81791 2.5578 0.212959 0.241214 0.248407 0.151878 0.0982524 0.224498 0.163714 470.907 8.44987 3.86514 0.308607 0.308607 0 6.38877 8.14244 14.3877
usedinputs                0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26
app_good_error_rate       0.5
error_rate                0.5
gen_bad_error_rate        0.5
gen_error_rate            0.5
gen_good_error_rate       0.5
global_gen_min_error      1
global_good_max_gen_error 0
global_good_min_gen_error 0.5
global_max_gen_error      0
global_min_gen_error      0.5
local_good_max_gen_error  0
local_good_min_gen_error  0.25
local_max_gen_error       0.25
local_min_gen_error       0.25
max_out                   1
min_out                   -1
steps_since_gen_min       0

*END

*BEGIN TREE fold_descriptor2
direction     -0.365918 0.740447 0.563775
e1e2          -0.59405 0.0665501 -0.1544
normal        0.917922 -0.190114 0.348246
nstats_E1E2   20
nstats_dir    20
nstats_normal 20

*END

*END

*END
