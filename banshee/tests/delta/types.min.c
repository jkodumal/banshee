struct typeExp
{
};
struct typeExp *
CreateType ()
{
  struct typeExp *tmp;
  tmp = (struct typeExp *) malloc (sizeof (struct typeExp));
  return (tmp);
}
struct typeExp *
HandleGeneric (struct typeExp *t)
{
  Substitute (CreateType ());
}
struct typeExp *
Substitute (struct typeExp *new)
{
}
