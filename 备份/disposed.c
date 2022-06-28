void
dispose_function_def_contents_self (c)
     FUNCTION_DEF *c;
{
  dispose_word_self (c->name);
  dispose_command (c->command);
  if(c->source_file){
      free(c->source_file);
  }
}
void
dispose_function_def_self (c)
     FUNCTION_DEF *c;
{
  dispose_function_def_contents_self (c);
  free (c);
}
void
dispose_word_self (w)
     WORD_DESC *w;
{
  if (w->word)
  {
      free(w->word);
  }
}
void
dispose_redirects_self (list)
     REDIRECT *list;
{
  register REDIRECT *t;

  while (list)
    {
      t = list;
      list = list->next;

      if (t->rflags & REDIR_VARASSIGN)
	dispose_word_self (t->redirector.filename);

      switch (t->instruction)
	{
	case r_reading_until:
	case r_deblank_reading_until:
	  free (t->here_doc_eof);
	/*FALLTHROUGH*/
	case r_reading_string:
	case r_output_direction:
	case r_input_direction:
	case r_inputa_direction:
	case r_appending_to:
	case r_err_and_out:
	case r_append_err_and_out:
	case r_input_output:
	case r_output_force:
	case r_duplicating_input_word:
	case r_duplicating_output_word:
	case r_move_input_word:
	case r_move_output_word:
	  dispose_word_self (t->redirectee.filename);
	  /* FALLTHROUGH */
	default:
	  break;
	}
      free (t);
    }
}
void
dispose_words_self (list)
     WORD_LIST *list;
{
  WORD_LIST *t;

  while (list)
    {
      t = list;
      list = list->next;
      dispose_word_self (t->word);
      free (t);
    }
}

void
dispose_cond_node_self (cond)
     COND_COM *cond;
{
  if (cond)
    {
      if (cond->left)
	dispose_cond_node_self (cond->left);
      if (cond->right)
	dispose_cond_node_self (cond->right);
      if (cond->op)
	dispose_word_self (cond->op);
      free (cond);
    }
}
void
dispose_command_self (command)
     COMMAND *command;
{
  if (command == 0)
    return;

  if (command->redirects)
    dispose_redirects_self (command->redirects);

  switch (command->type)
    {
    case cm_for:
#if defined (SELECT_COMMAND)
    case cm_select:
#endif
      {
	register FOR_COM *c;
#if defined (SELECT_COMMAND)
	if (command->type == cm_select)
	  c = (FOR_COM *)command->value.Select;
	else
#endif
	c = command->value.For;
	dispose_word_self (c->name);
	dispose_words_self (c->map_list);
	dispose_command_self (c->action);
	free (c);
	break;
      }

#if defined (ARITH_FOR_COMMAND)
    case cm_arith_for:
      {
	register ARITH_FOR_COM *c;

	c = command->value.ArithFor;
	dispose_words_self (c->init);
	dispose_words_self (c->test);
	dispose_words_self (c->step);
	dispose_command_self (c->action);
	free (c);
	break;
      }
#endif /* ARITH_FOR_COMMAND */

    case cm_group:
      {
	dispose_command_self (command->value.Group->command);
	free (command->value.Group);
	break;
      }

    case cm_subshell:
      {
	dispose_command_self (command->value.Subshell->command);
	free (command->value.Subshell);
	break;
      }

    case cm_coproc:
      {
	free (command->value.Coproc->name);
	dispose_command_self (command->value.Coproc->command);
	free (command->value.Coproc);
	break;
      }

    case cm_case:
      {
	register CASE_COM *c;
	PATTERN_LIST *t, *p;

	c = command->value.Case;
	dispose_word_self (c->word);

	for (p = c->clauses; p; )
	  {
	    dispose_words_self (p->patterns);
	    dispose_command_self (p->action);
	    t = p;
	    p = p->next;
	    free (t);
	  }
	free (c);
	break;
      }

    case cm_until:
    case cm_while:
      {
	register WHILE_COM *c;

	c = command->value.While;
	dispose_command_self (c->test);
	dispose_command_self (c->action);
	free (c);
	break;
      }

    case cm_if:
      {
	register IF_COM *c;

	c = command->value.If;
	dispose_command_self (c->test);
	dispose_command_self (c->true_case);
	dispose_command_self (c->false_case);
	free (c);
	break;
      }

    case cm_simple:
      {
	register SIMPLE_COM *c;

	c = command->value.Simple;
	dispose_words_self (c->words);
	dispose_redirects_self (c->redirects);
	free (c);
	break;
      }

    case cm_connection:
      {
	register CONNECTION *c;

	c = command->value.Connection;
	dispose_command_self (c->first);
	dispose_command_self (c->second);
	free (c);
	break;
      }

#if defined (DPAREN_ARITHMETIC)
    case cm_arith:
      {
	register ARITH_COM *c;

	c = command->value.Arith;
	dispose_words_self (c->exp);
	free (c);
	break;
      }
#endif /* DPAREN_ARITHMETIC */

#if defined (COND_COMMAND)
    case cm_cond:
      {
	register COND_COM *c;

	c = command->value.Cond;
	dispose_cond_node_self (c);
	break;
      }
#endif /* COND_COMMAND */

    case cm_function_def:
      {
	register FUNCTION_DEF *c;

	c = command->value.Function_def;
	dispose_function_def_self (c);
	break;
      }

    default:
    //   command_error ("dispose_command_self", CMDERR_BADTYPE, command->type, 0);
      break;
    }
  free (command);
}