ceval_summary_groups = []

_ceval_stem = ['computer_network', 'operating_system', 'computer_architecture', 'college_programming', 'college_physics', 'college_chemistry', 'advanced_mathematics', 'probability_and_statistics', 'discrete_mathematics', 'electrical_engineer', 'metrology_engineer', 'high_school_mathematics', 'high_school_physics', 'high_school_chemistry', 'high_school_biology', 'middle_school_mathematics', 'middle_school_biology', 'middle_school_physics', 'middle_school_chemistry', 'veterinary_medicine']
_ceval_stem = ['ceval-' + s for s in _ceval_stem]
ceval_summary_groups.append({'name': 'ceval-stem', 'subsets': _ceval_stem})

_ceval_social_science = ['college_economics', 'business_administration', 'marxism', 'mao_zedong_thought', 'education_science', 'teacher_qualification', 'high_school_politics', 'high_school_geography', 'middle_school_politics', 'middle_school_geography']
_ceval_social_science = ['ceval-' + s for s in _ceval_social_science]
ceval_summary_groups.append({'name': 'ceval-social-science', 'subsets': _ceval_social_science})

_ceval_humanities = ['modern_chinese_history', 'ideological_and_moral_cultivation', 'logic', 'law', 'chinese_language_and_literature', 'art_studies', 'professional_tour_guide', 'legal_professional', 'high_school_chinese', 'high_school_history', 'middle_school_history']
_ceval_humanities = ['ceval-' + s for s in _ceval_humanities]
ceval_summary_groups.append({'name': 'ceval-humanities', 'subsets': _ceval_humanities})

_ceval_other = ['civil_servant', 'sports_science', 'plant_protection', 'basic_medicine', 'clinical_medicine', 'urban_and_rural_planner', 'accountant', 'fire_engineer', 'environmental_impact_assessment_engineer', 'tax_accountant', 'physician']
_ceval_other = ['ceval-' + s for s in _ceval_other]
ceval_summary_groups.append({'name': 'ceval-other', 'subsets': _ceval_other})

_ceval_hard = ['advanced_mathematics', 'discrete_mathematics', 'probability_and_statistics', 'college_chemistry', 'college_physics', 'high_school_mathematics', 'high_school_chemistry', 'high_school_physics']
_ceval_hard = ['ceval-' + s for s in _ceval_hard]
ceval_summary_groups.append({'name': 'ceval-hard', 'subsets': _ceval_hard})

_ceval_all = _ceval_stem + _ceval_social_science + _ceval_humanities + _ceval_other
_ceval_weights = {'professional_tour_guide_val': 29, 'high_school_geography_val': 19, 'logic_val': 22, 'middle_school_politics_val': 21, 'college_chemistry_val': 24, 'electrical_engineer_val': 37, 'high_school_biology_val': 19, 'metrology_engineer_val': 24, 'high_school_history_val': 20, 'physician_val': 49, 'middle_school_physics_val': 19, 'marxism_val': 19, 'college_programming_val': 37, 'ideological_and_moral_cultivation_val': 19, 'teacher_qualification_val': 44, 'college_physics_val': 19, 'legal_professional_val': 23, 'computer_network_val': 19, 'middle_school_biology_val': 21, 'advanced_mathematics_val': 19, 'middle_school_chemistry_val': 20, 'middle_school_geography_val': 12, 'law_val': 24, 'college_economics_val': 55, 'mao_zedong_thought_val': 24, 'computer_architecture_val': 21, 'veterinary_medicine_val': 23, 'education_science_val': 29, 'art_studies_val': 33, 'middle_school_history_val': 22, 'clinical_medicine_val': 22, 'accountant_val': 49, 'chinese_language_and_literature_val': 23, 'modern_chinese_history_val': 23, 'probability_and_statistics_val': 18, 'civil_servant_val': 47, 'basic_medicine_val': 19, 'high_school_physics_val': 19, 'high_school_chemistry_val': 19, 'operating_system_val': 19, 'high_school_mathematics_val': 18, 'fire_engineer_val': 31, 'plant_protection_val': 22, 'discrete_mathematics_val': 16, 'environmental_impact_assessment_engineer_val': 31, 'high_school_chinese_val': 19, 'business_administration_val': 33, 'tax_accountant_val': 49, 'high_school_politics_val': 19, 'urban_and_rural_planner_val': 46, 'middle_school_mathematics_val': 19, 'sports_science_val': 19}
_ceval_weights = {'ceval-test-' + k : v for k,v in _ceval_weights.items()}
ceval_summary_groups.append({'name': 'ceval', 'subsets': _ceval_all})
ceval_summary_groups.append({'name': 'ceval-weighted', 'subsets': _ceval_all, 'weights': _ceval_weights})

_ceval_stem = ['computer_network', 'operating_system', 'computer_architecture', 'college_programming', 'college_physics', 'college_chemistry', 'advanced_mathematics', 'probability_and_statistics', 'discrete_mathematics', 'electrical_engineer', 'metrology_engineer', 'high_school_mathematics', 'high_school_physics', 'high_school_chemistry', 'high_school_biology', 'middle_school_mathematics', 'middle_school_biology', 'middle_school_physics', 'middle_school_chemistry', 'veterinary_medicine']
_ceval_stem = ['ceval-test-' + s for s in _ceval_stem]
ceval_summary_groups.append({'name': 'ceval-test-stem', 'subsets': _ceval_stem})

_ceval_social_science = ['college_economics', 'business_administration', 'marxism', 'mao_zedong_thought', 'education_science', 'teacher_qualification', 'high_school_politics', 'high_school_geography', 'middle_school_politics', 'middle_school_geography']
_ceval_social_science = ['ceval-test-' + s for s in _ceval_social_science]
ceval_summary_groups.append({'name': 'ceval-test-social-science', 'subsets': _ceval_social_science})

_ceval_humanities = ['modern_chinese_history', 'ideological_and_moral_cultivation', 'logic', 'law', 'chinese_language_and_literature', 'art_studies', 'professional_tour_guide', 'legal_professional', 'high_school_chinese', 'high_school_history', 'middle_school_history']
_ceval_humanities = ['ceval-test-' + s for s in _ceval_humanities]
ceval_summary_groups.append({'name': 'ceval-test-humanities', 'subsets': _ceval_humanities})

_ceval_other = ['civil_servant', 'sports_science', 'plant_protection', 'basic_medicine', 'clinical_medicine', 'urban_and_rural_planner', 'accountant', 'fire_engineer', 'environmental_impact_assessment_engineer', 'tax_accountant', 'physician']
_ceval_other = ['ceval-test-' + s for s in _ceval_other]
ceval_summary_groups.append({'name': 'ceval-test-other', 'subsets': _ceval_other})

_ceval_hard = ['advanced_mathematics', 'discrete_mathematics', 'probability_and_statistics', 'college_chemistry', 'college_physics', 'high_school_mathematics', 'high_school_chemistry', 'high_school_physics']
_ceval_hard = ['ceval-test-' + s for s in _ceval_hard]
ceval_summary_groups.append({'name': 'ceval-test-hard', 'subsets': _ceval_hard})

_ceval_all = _ceval_stem + _ceval_social_science + _ceval_humanities + _ceval_other
_ceval_test_weights = {'mao_zedong_thought_test': 219, 'modern_chinese_history_test': 212, 'legal_professional_test': 215, 'education_science_test': 270, 'high_school_biology_test': 175, 'chinese_language_and_literature_test': 209, 'accountant_test': 443, 'middle_school_chemistry_test': 185, 'plant_protection_test': 199, 'veterinary_medicine_test': 210, 'fire_engineer_test': 282, 'middle_school_biology_test': 192, 'high_school_mathematics_test': 166, 'high_school_chinese_test': 178, 'high_school_chemistry_test': 172, 'law_test': 221, 'middle_school_geography_test': 108, 'discrete_mathematics_test': 153, 'basic_medicine_test': 175, 'operating_system_test': 179, 'electrical_engineer_test': 339, 'college_programming_test': 342, 'marxism_test': 179, 'urban_and_rural_planner_test': 418, 'computer_architecture_test': 193, 'professional_tour_guide_test': 266, 'ideological_and_moral_cultivation_test': 172, 'high_school_physics_test': 175, 'clinical_medicine_test': 200, 'advanced_mathematics_test': 173, 'high_school_geography_test': 178, 'environmental_impact_assessment_engineer_test': 281, 'art_studies_test': 298, 'tax_accountant_test': 443, 'business_administration_test': 301, 'computer_network_test': 171, 'metrology_engineer_test': 219, 'logic_test': 204, 'probability_and_statistics_test': 166, 'high_school_politics_test': 176, 'middle_school_mathematics_test': 177, 'teacher_qualification_test': 399, 'physician_test': 443, 'civil_servant_test': 429, 'college_chemistry_test': 224, 'sports_science_test': 180, 'middle_school_history_test': 207, 'middle_school_politics_test': 193, 'college_physics_test': 176, 'college_economics_test': 497, 'high_school_history_test': 182, 'middle_school_physics_test': 178}
_ceval_test_weights = {'ceval-test-' + k : v for k,v in _ceval_test_weights.items()}
ceval_summary_groups.append({'name': 'ceval-test', 'subsets': _ceval_all})
ceval_summary_groups.append({'name': 'ceval-test-weighted', 'subsets': _ceval_all, 'weights': _ceval_test_weights})
