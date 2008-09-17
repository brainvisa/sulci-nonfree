# tree 1.0

*BEGIN TREE top_adaptive
nb_learn_data      3
significant_labels plop1 plop2
void_label         unknown
weight             5

*BEGIN TREE ad_leaf
learn_state   2
nb_learn_data 3
work          mlp_work1

*BEGIN TREE grid_optimizer
parameters	{
	'__syntax__' : 'grid_optimizer_parameters',
	'C' : {'ranges' : (1, 2, 10), 'scale' : 'log'},
	'gamma' : {'ranges' : (3, 5, 7), 'scale' : 'linear'}}
strategy	min
*END

*BEGIN TREE matrix_dimreduction
matrix	1. 2. 3. 4. 10. 20. 30. 40. 100. 200. 300. 400.
shape	3 4
select	2
*END

*BEGIN TREE sub_ad_mlp
app_good_error_rate       0.0857229
error_rate                0.0264594
gen_bad_error_rate        0.00136715
gen_error_rate            0.0178811
gen_good_error_rate       0.0837875
global_gen_min_error      0.0425773
global_good_max_gen_error 0.553359
global_good_min_gen_error 0.0837875
global_max_gen_error      0.510047
global_min_gen_error      0.0101579
local_good_max_gen_error  0.289976
local_good_min_gen_error  0.0837875
local_max_gen_error       0.0609498
local_min_gen_error       0.0145692
max_out                   1
min_out                   -1
steps_since_gen_min       0
eta                       0.15
mean                      1 1 -71.5991 4.90171 -24.1569 -5.401 44.4311 -75.0696 -43.9497 24.294 -52.7008 1 0.454884 -0.872027 -0.115139 -0.6728 -0.425274 0.590468 1235.35 23.2162 0 1 1 0 0 2.80952 399.43
name                      mlp_work1
net                       mlp.net
nstats                    21
sigma                     0 0 2.35189 4.42968 6.69698 5.87917 7.92869 12.59 3.42293 3.77657 3.45249 0 0.0674903 0.0332279 0.117187 0.0733338 0.0870237 0.0698532 200.847 1.53493 0 0 0 0 0 2.62985 74.594
usedinputs                0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26

*END

*BEGIN TREE fold_descriptor2
direction     1. 2. 3.
e1e2          4. 5. 6.
normal        7. 8. 9.
nstats_E1E2   21
nstats_dir    21
nstats_normal 21

*END

*END

*END
