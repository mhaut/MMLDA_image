/*
 * CTagLDA.h
 *
 *  Created on: Apr 10, 2010
 *      Author: data
 */

#ifndef CTAGLDA_H_
#define CTAGLDA_H_

#include "../DataSet/TagLDADataset.h"
#include "../DataSet/CParameterMatrixReaderWriter.h"
#include "AGraphicalModel.h"
#include "CTagLDAGeneralModel.h"
#include "../EMAlgorithms/CTagLDAEMFunctionoid.h"
#include "../Utilities/CUtilities.h"
#include "../Utilities/TemplateMath.h"
#include "../Utilities/cokus.h"

#define myrand_Cokus() (double) (((unsigned long) randomMT()) / 4294967296.0)


class CTagLDA : public CTagLDAGeneralModel {

protected:

	friend class CTagLDAEMFunctionoid;
	/*!
	 * @brief This is needed for printing topics as datum labels to file or console
	 */
	std::string _datum_to_id_dict_filename;
	/*!
	 * @brief This is needed for printing topics as tag labels to file or console
	 */
	std::string _tag_to_id_dict_filename;
	/*!
	 * @brief This member variable contains the absolute filename of the saved topic datum multinomial parameters
	 */
	std::string _model_train_saved_topic_tag_mult_filename;
	/*!
	 * @brief This member variable stores the root directory for the model
	 */
	std::string _model_root_dir;
	/*!
	 * @brief This member variable stores the training root directory for the model - useful during inference
	 */
	std::string _model_train_root_dir;
	/*!
	 * @brief full path for the model index file
	 */
	std::string _model_index_filename;
	/*!
	 * @brief This member variable indicates whether to read data in grouped format or sequential format.
	 * Default value is the std::string "grouped".
	 * Only two values are allowed - "grouped" or "sequential"
	 */
	std::string _data_input_mode;
	/*!
	 * The asymmetric dirichlet parameter file in case use of asymmetric dirichlet for topic proportions is set to true
	 */
	std::string _asym_alpha_initial_file;
	/*!
	 * If set to true, then the asym_alpha_initial_file is read
	 */
	bool _b_use_asym_alpha;
	/*!
	 * @brief a boolean to accomodate for unknown word ids
	 */
	bool _accommodate_for_unknown_datum;
	/*!
	 * @brief the total datum count in the training corpus - useful for measuring Bayesian Information Criterion
	 */
	long _training_datum_total_count;
	/*!
	 * @brief number of EM iterations
	 */
	int _n_em_iter;
	/*!
	 * @brief number of variational EM iterations
	 */
	int _n_em_var_iter;

	/*!
	 * @brief likelihood under this model for a given value of K and the set of tags
	 */
	double _log_prob;
	/*!
	 * @brief regularizer for the topic word parameter matrix update
	 */
	double _sigma_regl_topic_word;
	/*!
	 * @brief regularizer for the tag word parameter matrix update
	 */
	double _sigma_regl_tag_word;

protected:
	/*
	 * declaring the parameters of the model
	 */
	std::vector<std::vector<double>* >* _log_pi;		// this is the T x V matrix - model parameter
	std::vector<std::vector<double>* >* _log_beta;		// this is the K x V matrix - model parameter
	double _alpha;										// hyperparameter for topic proportions
	std::vector<double>* _asym_alpha;					// assymetric alpha for topic proportions

	/*
	 * sufficient statistics
	 */
	std::vector<std::vector<double>* >* _pi_ms_term1;	// this is the T x V matrix
	std::vector<std::vector<double>* >* _beta_ms_term1;	// this is the K x V matrix
	double _alpha_ss;									// hyperparameter for topic proportions

	std::vector<std::vector<double>* >* _pi_ms_term2;	// this is the T x V normalizer matrix for pi
	std::vector<std::vector<double>* >* _beta_ms_term2;	// this is the K x V normalizer matrix for beta

	std::vector<double> _beta_sum_vec;
	std::vector<double> _pi_sum_vec;
	std::vector<std::vector<double>* >* _beta_pi_sum_mat;	// this is the K x T matrix
	std::vector<double>* _asym_alpha_ss;					// assymetric alpha for topic proportions
	std::vector<double>* _asym_alpha_exp_ss;		// initializer in optimization of assymetric alpha for topic proportions
	std::vector<double>* _asym_alpha_exp_square_ss;	// initializer in optimization of assymetric alpha for topic proportions
	double _asym_alpha_concentration_parameter;				// concentration parameter for alpha - set to 10 in the constructor for now

	STagLDACorpus* _corpus;

	size_t _D;
	size_t _V;
	size_t _K;
	size_t _T;
	size_t _n_max_terms_in_a_doc;

	/*
	 * variational matrices and hyperparameters (updated per document)
	 */
	std::vector<double>* 				_var_zeta_perdoc;	// this is the N x 1 zeta variational param matrix for each word in the doc
	std::vector<std::vector<double>* >* _var_phi;			// this is the N x K matrix

	std::vector<std::vector<double>* >* _var_gamma;			// D x K matrix variational hyper parameter for topic proportions

	bool _estimate_hypparam;

public:
	CTagLDA();
	~CTagLDA();

	inline STagLDACorpus* get_corpus() { return _corpus; }

	// mutator
	inline void set_n_topics(int n_topics) { _K = n_topics; }
	inline void set_n_tags(int n_tags) { _T = n_tags; }

	// getters and setters
	/**
	 *
	 */
	virtual void set_bytecode(std::string bytecode) { CTagLDAGeneralModel::_bytecode = bytecode; }
	/**
	 *
	 */
	inline std::string get_bytecode() { return _bytecode; }
	/**
	 *
	 */
	virtual void set_model_dump_mode(std::string model_dump_mode) { CTagLDAGeneralModel::_model_dump_mode = model_dump_mode; }
	/**
	 *
	 */
	virtual void set_model_operation_mode(std::string model_operation_mode) { CTagLDAGeneralModel::_model_operation_mode = model_operation_mode; }
	/**
	 *
	 */
	virtual std::string get_model_operation_mode() { return CTagLDAGeneralModel::_model_operation_mode; }
	/**
	 *
	 */
	virtual std::string get_model_dump_mode() { return CTagLDAGeneralModel::_model_dump_mode; }
	/**
	 *
	 */
	virtual std::string get_model_root_dir() { return _model_root_dir; }
	/**
	 *
	 */
	virtual inline std::string to_string() {
		if ( CTagLDAGeneralModel::_model_operation_mode == "train" ) return "TagLDA-train";
		if ( CTagLDAGeneralModel::_model_operation_mode == "test" || CTagLDAGeneralModel::_model_operation_mode == "validate" ) return "TagLDA-test";
		return "TagLDA-EM";	// returns this std::string by default
	}
	/**
	 *
	 */
	virtual std::vector<std::vector<double>* >* get_topic_datum_multinomials() { return _log_beta; }
	/**
	 *
	 */
	virtual std::vector<std::vector<double>* >* get_tag_datum_multinomials() { return _log_pi; }
	/**
	 *
	 */
	virtual std::vector<std::vector<double>* >* get_topic_proportion_multinomials() { return _var_gamma; }
	/**
	 *
	 */
	virtual std::string get_datum_to_id_dictionary_filename() { return _datum_to_id_dict_filename; }
	/**
	 *
	 */
	virtual std::string get_tag_to_id_dictionary_filename() { return _tag_to_id_dict_filename; }
	/**
	 *
	 */
	virtual std::string get_saved_topic_tag_multinomial_filename() { return _model_train_saved_topic_tag_mult_filename; }
	/**
	 *
	 */
	virtual size_t get_doc_count() { return _D; }
	/**
	 *
	 */
	virtual size_t get_vocabulary_size() { return _V; }
	/**
	 *
	 */
	virtual size_t get_tag_vocabulary_size() { return _T; }
	/**
	 * @return a long integer indicating the total number of terms in the training corpus
	 */
	virtual long get_total_datum_count() { return _training_datum_total_count; }

	// other methods
	/**
	 * @brief
	 */
	void corpus_initialize();
	/**
	 * @brief
	 */
	void random_initialize();
	/**
	 * @brief returns a boolean indicating if the model can be loaded
	 * @return a boolean indicating if the model can be loaded
	 */
	virtual bool load_model();
	/**
	 * @brief
	 */
	virtual void save_model();
	/**
	 *
	 */
	virtual void read_data_from_file();
	/**
	 *
	 */
	virtual void read_config_file(std::string model_config_filename);
	/**
	 *
	 */
	virtual bool create_and_initialize_data_structures();
	/**
	 *
	 */
	virtual bool zero_initialize_suff_stats();
	/**
	 *
	 */
	virtual void reset_data_structures();
	/**
	 *
	 */
	virtual double compute_log_likelihood();

protected:
	/**
	 *
	 */
	void read_data_from_file_for_training();
	/**
	 *
	 */
	void read_data_from_file_for_testing();
	/**
	 *
	 */
	bool create_and_initialize_data_structures_for_parameters();
	/**
	 *
	 */
	bool create_and_initialize_data_structures_for_hidden_variables_and_suff_stats();
	/**
	 *
	 */
	double compute_doc_log_likelihood(size_t doc_id, STagLDADocument* doc);
	/**
	 *
	 */
	void precompute_beta_and_pi_row_sums();

public:
	/**
	 *  some diagnostic functions
	 */
	virtual void print_parameters();
};

#endif /* CTAGLDA_H_ */
