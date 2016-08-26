#include "rose.h"
#include "autoParSupport.h"

#define STR_BUF 256
#define COMP_MSG_LINES 1024*1024

using namespace std;
using namespace OmpSupport;
using namespace AutoParallelization;

enum MSG_TYPE
  {
    error, warn, ext, obsol, del, vec, opt, observ, mul, others,
  };

typedef struct
{
  char lang[STR_BUF];
  int msg_id;
  char file_name[STR_BUF];
  int line;  
  int id;
} COMP_MSG;

typedef struct
{
  COMP_MSG *comp_msg[10];
  int msg_cnt[10];
} COMP_LOG;

void read_comp_log( COMP_LOG *comp_log )
{
  int i;
  int index;
  FILE *fp;
  char string_buf[STR_BUF], lang[STR_BUF], type[STR_BUF], file_name[STR_BUF], dummy[STR_BUF];
  int msg_id, line;
  MSG_TYPE msg_type;

  for( i = 0 ; i < 10 ; i ++ ) comp_log->msg_cnt[i] = 0;
  
  if( NULL == (fp = fopen( "compile_log.txt", "r" )))
    {
      cerr<<"file open error\n";
      exit(1);
    }
  while( fgets( string_buf, STR_BUF, fp ))
    {
      if( !strncmp( string_buf, "#", 1 )) continue;
      else if( !strncmp( string_buf, "f90", 3 ) || !strncmp( string_buf, "sxc++", 5 ) || !strncmp( string_buf, "sxcc", 4 ))
	{
	  sscanf( string_buf, "%s %[^(](", lang, type );
	  if( !strncmp( type, "error", 5 ))       msg_type = error;
	  else if( !strncmp( type, "warn", 4 ))   msg_type = warn;
	  else if( !strncmp( type, "ext", 3 ))    msg_type = ext;
	  else if( !strncmp( type, "obsol", 5 ))  msg_type = obsol;
	  else if( !strncmp( type, "del", 3 ))    msg_type = del;
	  else if( !strncmp( type, "vec", 3 ))    msg_type = vec;
	  else if( !strncmp( type, "opt", 3 ))    msg_type = opt;
	  else if( !strncmp( type, "observ", 6 )) msg_type = observ;
	  else if( !strncmp( type, "mul", 3 ))    msg_type = mul;
	  else                                    msg_type = others;
	  comp_log->msg_cnt[msg_type] ++;
	}
    }

  for( i = 0 ; i < 10 ; i ++ )
    {
      comp_log->comp_msg[i] = (COMP_MSG *)malloc( comp_log->msg_cnt[i] * sizeof(COMP_MSG));
      comp_log->msg_cnt[i] = 0;
    }
  
  fseek( fp, 0, SEEK_SET );
  
  while( fgets( string_buf, STR_BUF, fp ))
    {
      if( !strncmp( string_buf, "#", 1 )) continue;
      else if(( !strncmp( string_buf, "f90", 3 )) || ( !strncmp( string_buf, "sxc++", 5 )) || !strncmp( string_buf, "sxcc", 4 ))
  	{
  	  sscanf( string_buf, "%s %[^(](%d %s %[^,], %s %d", lang, type, &msg_id, dummy, file_name, dummy, &line );
	  if( !strncmp( type, "error", 5 ))       msg_type = error;
	  else if( !strncmp( type, "warn", 4 ))   msg_type = warn;
	  else if( !strncmp( type, "ext", 3 ))    msg_type = ext;
	  else if( !strncmp( type, "obsol", 5 ))  msg_type = obsol;
	  else if( !strncmp( type, "del", 3 ))    msg_type = del;
	  else if( !strncmp( type, "vec", 3 ))    msg_type = vec;
	  else if( !strncmp( type, "opt", 3 ))    msg_type = opt;
	  else if( !strncmp( type, "observ", 6 )) msg_type = observ;
	  else if( !strncmp( type, "mul", 3 ))    msg_type = mul;
	  else                                    msg_type = others;

	  index = comp_log->msg_cnt[msg_type];
	  strncpy( comp_log->comp_msg[msg_type][index].lang, lang, STR_BUF );
	  comp_log->comp_msg[msg_type][index].msg_id = msg_id;
	  strncpy( comp_log->comp_msg[msg_type][index].file_name, file_name, STR_BUF );
	  comp_log->comp_msg[msg_type][index].line = line;
	  comp_log->comp_msg[msg_type][index].id = index;  
	  comp_log->msg_cnt[msg_type] ++;
  	}
    }
  fclose(fp);

  return;
}

int main ( int argc, char* argv[] )
{
  int i, j;

  if(argc <= 1 )
    {
      cout<<"prepair: Please save a compile message as compile_log.txt\n";
      cout<<"usage: ./autoOMP [source file]\n";
      exit(1);
    }
  
  COMP_LOG comp_log;
  read_comp_log( &comp_log );

  SgProject* project = frontend(argc,argv);

  // Prepare liveness analysis
  initialize_analysis(project, false); // debug=true/false

  SgFilePtrList & ptr_list = project->get_fileList();
  for (SgFilePtrList::iterator iter = ptr_list.begin(); iter!=ptr_list.end();
       iter++)
    {
      SgFile* sageFile = (*iter);
      SgSourceFile * sfile = isSgSourceFile(sageFile);
      ROSE_ASSERT(sfile);
      SgGlobal *root = sfile->get_globalScope();
      SgDeclarationStatementPtrList& declList = root->get_declarations ();

      //For each function body in the scope
      for (SgDeclarationStatementPtrList::iterator p = declList.begin(); p != declList.end(); ++p) 
	{
	  SgFunctionDeclaration *func = isSgFunctionDeclaration(*p);
	  if (func == 0)  continue;
	  SgFunctionDefinition *defn = func->get_definition();
	  if (defn == 0)  continue;
	  if (defn->get_file_info()->get_filename()!=sageFile->get_file_info()->get_filename())
	    continue;
	  SgBasicBlock *body = defn->get_body();  

	  // X. Replace operators with their equivalent counterparts defined in "inline" annotations
	  AstInterfaceImpl faImpl_1(body);
	  CPPAstInterface fa_body(&faImpl_1);
	  OperatorInlineRewrite()( fa_body, AstNodePtrImpl(body));

	  // Pass annotations to arrayInterface and use them to collect alias info. function info etc.  
	  ArrayAnnotation* annot = ArrayAnnotation::get_inst(); 
	  ArrayInterface array_interface(*annot);
	  array_interface.initialize(fa_body, AstNodePtrImpl(defn));
	  array_interface.observe(fa_body);

	  // X. Loop normalization for all loops within body
	  Rose_STL_Container<SgNode*> loops = NodeQuery::querySubTree(defn,V_SgForStatement);
	  if (0==loops.size()) continue;	  


	  for (Rose_STL_Container<SgNode*>::iterator iter = loops.begin(); iter!= loops.end(); iter++ ) 
	    {
	      SgNode* current_loop = *iter;
	      OmpSupport::OmpAttribute* attribute = buildOmpAttribute(e_unknown, NULL, false);

	      int line = current_loop->get_file_info()->get_line();

	      for( j = 0 ; j < comp_log.msg_cnt[mul] ; j++ )
		{
		  if (( line == comp_log.comp_msg[mul][j].line ) && ( 1 == comp_log.comp_msg[mul][j].msg_id ))
		    {
		      if( current_loop->variantT()==V_SgForStatement )
			{
			  SgForStatement* forstmt = isSgForStatement(*(iter));
			  ROSE_ASSERT(forstmt != NULL);

			  //X. Parallelize loop one by one
			  SgInitializedName* invarname = getLoopInvariant(current_loop);
			  if (invarname != NULL)
			    {
			      //hasOpenMP = ParallelizeOutermostLoop(current_loop, &array_interface, annot);
			      std::map<SgNode*, bool> indirect_array_table;
			      if (b_unique_indirect_index) // uniform and collect indirect indexed array only when needed
				{
				  // uniform array reference expressions
				  uniformIndirectIndexedArrayRefs(isSgForStatement(current_loop));
				  collectIndirectIndexedArrayReferences (current_loop, indirect_array_table);
				}

			      LoopTreeDepGraph* depgraph= ComputeDependenceGraph(current_loop, &array_interface, annot);

			      // Variable liveness analysis: original ones and 
			      // the one containing only variables with some kind of dependencies
			      std::vector<SgInitializedName*> liveIns0, liveIns;
			      std::vector<SgInitializedName*> liveOuts0, liveOuts;
			      // Turn on recomputing since transformations have been done
			      //GetLiveVariables(sg_node,liveIns,liveOuts,true);
			      GetLiveVariables(current_loop,liveIns0,liveOuts0,false);
			      // Remove loop invariant variable, which is always private 
			      SgInitializedName* invarname = getLoopInvariant(current_loop);
			      remove(liveIns0.begin(),liveIns0.end(),invarname);
			      remove(liveOuts0.begin(),liveOuts0.end(),invarname);

			      std::vector<SgInitializedName*> allVars,depVars, invariantVars, privateVars,lastprivateVars, 
				firstprivateVars,reductionVars, reductionResults;
			      // Only consider scalars for now
			      CollectVisibleVaribles(current_loop,allVars,invariantVars,true);
			      CollectVariablesWithDependence(current_loop,depgraph,depVars,true);
			      if (enable_debug)
				{
				  cout<<"Debug after CollectVariablesWithDependence():"<<endl;
				  for (std::vector<SgInitializedName*>::iterator iter = depVars.begin(); iter!= depVars.end();iter++)
				    {
				      cout<<(*iter)<<" "<<(*iter)->get_qualified_name().getString()<<endl;
				    }
				}
			      sort(liveIns0.begin(), liveIns0.end());
			      sort(liveOuts0.begin(), liveOuts0.end());

			      //Remove the live variables which have no relevant dependencies
			      set_intersection(liveIns0.begin(),liveIns0.end(), depVars.begin(), depVars.end(),
					       inserter(liveIns, liveIns.begin()));
			      set_intersection(liveOuts0.begin(),liveOuts0.end(), depVars.begin(), depVars.end(),
					       inserter(liveOuts, liveOuts.begin()));

			      sort(liveIns.begin(), liveIns.end());
			      sort(liveOuts.begin(), liveOuts.end());
			      // shared: scalars for now: allVars - depVars, 

			      //private:
			      //---------------------------------------------
			      //depVars- liveIns - liveOuts
			      std::vector<SgInitializedName*> temp;
			      set_difference(depVars.begin(),depVars.end(), liveIns.begin(), liveIns.end(),
					     inserter(temp, temp.begin()));
			      set_difference(temp.begin(),temp.end(), liveOuts.begin(), liveOuts.end(),
					     inserter(privateVars, privateVars.end()));	
			      for(std::vector<SgInitializedName*>::iterator iter =invariantVars.begin();
				  iter!=invariantVars.end(); iter++)
				privateVars.push_back(*iter);

			      cout<<"Debug dump private:"<<endl;
			      for (std::vector<SgInitializedName*>::iterator iter = privateVars.begin(); iter!= privateVars.end();iter++) 
				{
				  attribute->addVariable(OmpSupport::e_private ,(*iter)->get_name().getString(), *iter);
				  cout<<(*iter)<<" "<<(*iter)->get_qualified_name().getString()<<endl;
				}

			      //lastprivate: liveOuts - LiveIns 
			      // Must be written and LiveOut to have the need to preserve the value:  DepVar Intersect LiveOut
			      // Must not be Livein to ensure correct semantics: private for each iteration, not getting value from previous iteration.
			      //  e.g.  for ()   {  a = 1; }  = a; 
			      //---------------------------------------------
			      set_difference(liveOuts.begin(), liveOuts.end(), liveIns0.begin(), liveIns0.end(),
					     inserter(lastprivateVars, lastprivateVars.begin()));

			      cout<<"Debug dump lastprivate:"<<endl;
			      for (std::vector<SgInitializedName*>::iterator iter = lastprivateVars.begin(); iter!= lastprivateVars.end();iter++) 
				{
				  attribute->addVariable(OmpSupport::e_lastprivate ,(*iter)->get_name().getString(), *iter);
				  cout<<(*iter)<<" "<<(*iter)->get_qualified_name().getString()<<endl;
				}
      
			      reductionResults = RecognizeReduction(current_loop,attribute, liveIns);

			      // firstprivate:  liveIns - LiveOuts - writtenVariables (or depVars)
			      //---------------------------------------------
			      //     liveIn : the need to pass in value
			      //     not liveOut: differ from Shared, we considered shared first, then firstprivate
			      //     not written: ensure the correct semantics: each iteration will use a copy from the original master, not redefined
			      //                  value from the previous iteration
			      std::vector<SgInitializedName*> temp2;
			      set_difference(liveIns0.begin(), liveIns0.end(), liveOuts0.begin(),liveOuts0.end(),
					     inserter(temp2, temp2.begin()));
			      set_difference(temp2.begin(), temp2.end(), depVars.begin(), depVars.end(),
					     inserter(firstprivateVars, firstprivateVars.begin()));
			      cout<<"Debug dump firstprivate:"<<endl;
			      for (std::vector<SgInitializedName*>::iterator iter = firstprivateVars.begin(); iter!= firstprivateVars.end();iter++) 
				{
				  attribute->addVariable(OmpSupport::e_firstprivate ,(*iter)->get_name().getString(), *iter);
				  cout<<(*iter)<<" "<<(*iter)->get_qualified_name().getString()<<endl;
				}

			      //X. Eliminate irrelevant dependence relations.
			      vector<DepInfo>  remainingDependences;
			      DependenceElimination(current_loop, depgraph, remainingDependences,attribute, indirect_array_table, &array_interface, annot);
			    }

			  //Collect reduction variables and operations
			  std::set<  std::pair <SgInitializedName*, VariantT> > reductions;
			  std::set<  std::pair <SgInitializedName*, VariantT> >::const_iterator iter;
			  SageInterface::ReductionRecognition(forstmt, reductions);

			  string omp_reduction_clause;
			  for (iter=reductions.begin(); iter!=reductions.end(); iter++)
			  {
			  std::pair <SgInitializedName*, VariantT> item = *iter;

			  switch(item.second)
			  {
			  case V_SgPlusPlusOp:
			  case V_SgPlusAssignOp:
			  case V_SgAddOp:
			  {
			  omp_reduction_clause += " reduction(+:" + item.first->unparseToString() + ")";
			  attribute->addVariable(OmpSupport::e_reduction_plus, item.first->get_name().getString(),item.first);
			  break;
			  }
			  case V_SgMinusMinusOp:
			  case V_SgMinusAssignOp:
			  case V_SgSubtractOp:
			  {
			  omp_reduction_clause += " reduction(-:" + item.first->unparseToString() + ")";
			  attribute->addVariable(OmpSupport::e_reduction_minus, item.first->get_name().getString(),item.first);
			  break;
			  }
			  case V_SgMultAssignOp:
			  case V_SgMultiplyOp:
			  {
			  omp_reduction_clause += " reduction(*:" + item.first->unparseToString() + ")";
			  attribute->addVariable(OmpSupport::e_reduction_mul, item.first->get_name().getString(),item.first);
			  break;
			  }
			  case V_SgAndAssignOp:
			  case V_SgBitAndOp:
			  {
			  omp_reduction_clause += " reduction(&:" + item.first->unparseToString() + ")";
			  attribute->addVariable(OmpSupport::e_reduction_bitand, item.first->get_name().getString(),item.first);
			  break;
			  }
			  case V_SgXorAssignOp:
			  case V_SgBitXorOp:
			  {
			  omp_reduction_clause += " reduction(^:" + item.first->unparseToString() + ")";
			  attribute->addVariable(OmpSupport::e_reduction_bitxor, item.first->get_name().getString(),item.first);
			  break;
			  }
			  case V_SgIorAssignOp:
			  case V_SgBitOrOp:
			  {
			  omp_reduction_clause += " reduction(|:" + item.first->unparseToString() + ")";
			  attribute->addVariable(OmpSupport::e_reduction_bitor, item.first->get_name().getString(),item.first);
			  break;
			  }
			  case V_SgAndOp:
			  {
			  omp_reduction_clause += " reduction(&&:" + item.first->unparseToString() + ")";
			  attribute->addVariable(OmpSupport::e_reduction_and, item.first->get_name().getString(),item.first);
			  break;
			  }
			  case V_SgOrOp:
			  {
			  omp_reduction_clause += " reduction(||:" + item.first->unparseToString() + ")";
			  attribute->addVariable(OmpSupport::e_reduction_or, item.first->get_name().getString(),item.first);
			  break;
			  }
			  default:
			  break;
			  }
			  }
			}
		      attribute->setOmpDirectiveType(OmpSupport::e_parallel_for);
		      OmpSupport::addOmpAttribute(attribute,current_loop);
		      comp_log.comp_msg[mul][j].msg_id = 0;
		      cout<<"Semi-automatic insertion to a loop at line:"<<current_loop->get_file_info()->get_line()<<endl;
		    }
		}
	      OmpSupport::generatePragmaFromOmpAttribute(current_loop);
	    }
	}
    }

  for( i = 0 ; i < 10 ; i ++ )
    {
      free(comp_log.comp_msg[i]);
    }

  if (liv !=NULL) 
    delete liv;

  return backend(project);
}
