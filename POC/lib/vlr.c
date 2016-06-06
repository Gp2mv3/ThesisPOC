//see Boneh, Boyen and Shacham, "Short Group Signatures"
#include <pbc/pbc.h>
#include "inc/vlr.h"
#include "inc/hash.h"

void vlr_gen_sys_param(vlr_sys_param_ptr param, pairing_ptr pairing)
{
	param->pairing = pairing;
	param->signature_length = 3 * pairing->G1->fixed_length_in_bytes + 6 * pairing->Zr->fixed_length_in_bytes;
}

void vlr_gen(vlr_group_public_key_ptr gpk, vlr_group_revocation_token_t *grt, int n, vlr_user_private_key_t *usk, vlr_sys_param_ptr param)
{
	pairing_ptr pairing = param->pairing;
	element_t z0;
	element_t gamma;
	int i;

	gpk->param = param;

	element_init_G1(gpk->g1, pairing);
	element_init_G2(gpk->g2, pairing);
	element_init_G2(gpk->omega, pairing);
	element_init_Zr(gamma, pairing);
	element_init_Zr(z0, pairing);

	element_random(gpk->g2);
	element_random(gpk->g1);
	element_random(gamma);

	element_pow_zn(gpk->omega, gpk->g2, gamma);

	for (i=0; i<n; i++) {
		usk[i]->param = param;
		grt[i]->param = param;

		element_init_G1(usk[i]->A, pairing);
		element_init_Zr(usk[i]->x, pairing);

		element_random(usk[i]->x);
		element_add(z0, gamma, usk[i]->x);
		element_invert(z0, z0);
		element_pow_zn(usk[i]->A, gpk->g1, z0);

		memcpy(grt[i]->A, usk[i]->A, sizeof(usk[i]->A));
	}

	element_clear(z0);
	element_clear(gamma);
}

void vlr_sign(unsigned char *sig, int hashlen, void *hash, vlr_group_public_key_ptr gpk, vlr_user_private_key_ptr usk)
{
	vlr_sys_param_ptr param = gpk->param;
	pairing_ptr pairing = param->pairing;
	field_ptr Fp = pairing->Zr;

	element_t M;
	element_init_G1(M, pairing);
	element_from_hash(M, hash, hashlen);


	element_t r;

	element_t u, v;
	element_t alpha, delta;

	element_t T1, T2;
	element_t R1, R2, R3;
	element_t temp1;

	element_t c;
	element_t ralpha, rx, rdelta;


	element_t z0, z1;
	element_t et;
	unsigned char *writeptr = sig;

	element_init_G1(T1, pairing);
	element_init_G1(T2, pairing);

	element_init_G1(R1, pairing);
	element_init_GT(R2, pairing);
	element_init_G1(R3, pairing);

	element_init_G1(temp1, pairing);

	element_init_G1(u, pairing);
	element_init_G1(v, pairing);
	element_init_Zr(r, pairing);
	element_random(r);

	element_init(c, Fp);
	element_init(alpha, Fp); element_random(alpha);
	element_init(delta, Fp);

	//temp variables
	element_init(z0, Fp);
	element_init(z1, Fp);
	element_init_GT(et, pairing);


	unsigned int hash_input_length = element_length_in_bytes(gpk->g1) +
	element_length_in_bytes(gpk->g2) +
	element_length_in_bytes(gpk->omega) +
	element_length_in_bytes(r) +
	element_length_in_bytes(M);

	unsigned char *hash_input = malloc(hash_input_length);

	hash_input += element_to_bytes(hash_input, gpk->g1);
	hash_input += element_to_bytes(hash_input, gpk->g2);
	hash_input += element_to_bytes(hash_input, gpk->omega);
	hash_input += element_to_bytes(hash_input, M); // Could avoid converting to bytes and from bytes
	hash_input += element_to_bytes(hash_input, r);
	hash_input -= hash_input_length;

	hash_ctx_t context;
	unsigned char digest[hash_length];

	hash_init(context);
	hash_update(context, hash_input, hash_input_length);
	hash_final(digest, context);

	element_from_hash(u, digest, sizeof(digest));

	//TODO: v is equal to u, find something more interresting here !
	element_from_hash(v, digest, sizeof(digest));


	element_pow_zn(T1, u, alpha);
	element_pow_zn(temp1, v, alpha);
	element_mul(T2, usk->A, temp1);

	element_mul(delta, usk->x, alpha);


	element_init(ralpha, Fp); element_random(ralpha);
	element_init(rx, Fp); element_random(rx);
	element_init(rdelta, Fp); element_random(rdelta);



	element_pow_zn(R1, u, ralpha);

	//R3: seems correct
	element_pow_zn(R3, T1, rx);
	element_neg(z0, rdelta);
	element_pow_zn(temp1, u, z0); //u^{-r_alpha}
	element_mul(R3, R3, temp1);

	//R2: Seems correct now
	pairing_apply(R2, T2, gpk->g2, pairing);
	element_pow_zn(R2, R2, rx);

	element_neg(z0, ralpha);
	pairing_apply(et, v, gpk->omega, pairing);
	element_pow_zn(et, et, z0);
	element_mul(R2, R2, et);

	element_neg(z0, rdelta);
	pairing_apply(et, v, gpk->g2, pairing);
	element_pow_zn(et, et, z0);
	element_mul(R2, R2, et);

	//TODO: Better use element_pow3_zn => faster


	hash_input_length = element_length_in_bytes(gpk->g1) +
	element_length_in_bytes(gpk->g2) +
	element_length_in_bytes(gpk->omega) +
	element_length_in_bytes(M) +
	element_length_in_bytes(r) +
	element_length_in_bytes(T1) +
	element_length_in_bytes(T2) +
	element_length_in_bytes(R1) +
	element_length_in_bytes(R2) +
	element_length_in_bytes(R3);

	free(hash_input);
	hash_input = malloc(hash_input_length);

	hash_input += element_to_bytes(hash_input, gpk->g1);
	hash_input += element_to_bytes(hash_input, gpk->g2);
	hash_input += element_to_bytes(hash_input, gpk->omega);
	hash_input += element_to_bytes(hash_input, M); // Could avoid converting to bytes and from bytes
	hash_input += element_to_bytes(hash_input, r);
	hash_input += element_to_bytes(hash_input, T1);
	hash_input += element_to_bytes(hash_input, T2);
	hash_input += element_to_bytes(hash_input, R1);
	hash_input += element_to_bytes(hash_input, R2);
	hash_input += element_to_bytes(hash_input, R3);
	hash_input -= hash_input_length;

	hash_init(context);
	hash_update(context, hash_input, hash_input_length);
	hash_final(digest, context);
	free(hash_input);

	element_from_hash(c, digest, sizeof(digest));

	#if DEBUG
	element_printf("ralpha: %B\n", ralpha);
	element_printf("rx: %B\n", rx);
	element_printf("rdelta: %B\n", rdelta);


	element_printf("alpha: %B\n", alpha);
	element_printf("x: %B\n", usk->x);
	element_printf("delta: %B\n", delta);
	#endif

	//now the r's represent the values of the s's
	//no need to allocate yet more variables
	element_mul(z0, c, alpha);
	element_add(ralpha, ralpha, z0);

	element_mul(z0, c, usk->x);
	element_add(rx, rx, z0);


	element_mul(z0, c, delta);
	element_add(rdelta, rdelta, z0);

	writeptr += element_to_bytes(writeptr, r);
	writeptr += element_to_bytes(writeptr, T1);
	writeptr += element_to_bytes(writeptr, T2);
	writeptr += element_to_bytes(writeptr, c);
	writeptr += element_to_bytes(writeptr, ralpha);
	writeptr += element_to_bytes(writeptr, rx);
	writeptr += element_to_bytes(writeptr, rdelta);

	#ifdef DEBUG
	element_printf("T1: %B\n", T1);
	element_printf("T2: %B\n", T2);
	element_printf("R1: %B\n", R1);
	element_printf("R2: %B\n", R2);
	element_printf("R3: %B\n", R3);

	element_printf("M: %B\n", M);

	element_printf("c: %B\n", c);

	element_printf("salpha: %B\n", ralpha);
	element_printf("sx: %B\n", rx);
	element_printf("sdelta: %B\n", rdelta);
	#endif

	element_clear(T1);
	element_clear(T2);
	element_clear(R1);
	element_clear(R2);
	element_clear(R3);
	element_clear(alpha);
	element_clear(delta);

	element_clear(r);
	element_clear(c);
	element_clear(u);
	element_clear(v);

	element_clear(ralpha);
	element_clear(rx);
	element_clear(rdelta);
	//clear temp variables
	element_clear(z0);
	element_clear(z1);

	element_clear(et);
	element_clear(temp1);

	element_clear(M);
}

int vlr_verify(unsigned char *sig, int hashlen, void *hash, vlr_group_public_key_t gpk, vlr_revocation_list_t *RL, int RLlen)
{
	vlr_sys_param_ptr param = gpk->param;
	pairing_ptr pairing = param->pairing;
	field_ptr Fp = pairing->Zr;
	element_t r;
	element_t T1, T2;
	element_t R1, R2, R3;
	element_t u, v;
	element_t c, c2, salpha, sx, sdelta;
	element_t e0;
	element_t z0, zt, zt2;

	element_t M;
	element_init_G1(M, pairing);
	element_from_hash(M, hash, hashlen);

	unsigned char *readptr = sig;

	element_init_G1(T1, pairing);
	element_init_G1(T2, pairing);
	element_init_G1(R1, pairing);
	element_init_GT(R2, pairing);
	element_init_G1(R3, pairing);

	element_init_G1(u, pairing);
	element_init_G1(v, pairing);

	element_init(c, Fp);
	element_init(c2, Fp);

	element_init(salpha, Fp);
	element_init(sx, Fp);
	element_init(sdelta, Fp);
	element_init(r, Fp);

	// Temp variables
	element_init_G1(z0, pairing);
	element_init_GT(zt, pairing);
	element_init_GT(zt2, pairing);
	element_init(e0, Fp);


	// Read signature elements
	readptr += element_from_bytes(r, readptr);
	readptr += element_from_bytes(T1, readptr);
	readptr += element_from_bytes(T2, readptr);
	readptr += element_from_bytes(c, readptr);
	readptr += element_from_bytes(salpha, readptr);
	readptr += element_from_bytes(sx, readptr);
	readptr += element_from_bytes(sdelta, readptr);


	//Hash to recover u and v
	unsigned int hash_input_length = element_length_in_bytes(gpk->g1) +
	element_length_in_bytes(gpk->g2) +
	element_length_in_bytes(gpk->omega) +
	element_length_in_bytes(M) +
	element_length_in_bytes(r);

	unsigned char *hash_input = malloc(hash_input_length);
	hash_input += element_to_bytes(hash_input, gpk->g1);
	hash_input += element_to_bytes(hash_input, gpk->g2);
	hash_input += element_to_bytes(hash_input, gpk->omega);
	hash_input += element_to_bytes(hash_input, M); // Could avoid converting to bytes and from bytes
	hash_input += element_to_bytes(hash_input, r);
	hash_input -= hash_input_length;

	hash_ctx_t context;
	unsigned char digestUV[hash_length];

	hash_init(context);
	hash_update(context, hash_input, hash_input_length);
	hash_final(digestUV, context);
	free(hash_input);

	element_from_hash(u, digestUV, sizeof(digestUV));

	//TODO: v is equal to u, find something more interresting here !
	element_from_hash(v, digestUV, sizeof(digestUV));


	element_pow_zn(R1, u, salpha);
	element_pow_zn(z0, T1, c);
	element_div(R1, R1, z0);


	element_pow_zn(R3, T1, sx);
	element_neg(e0, sdelta);
	element_pow_zn(z0, u, e0);
	element_mul(R3, R3, z0);

	pairing_apply(R2, T2, gpk->g2, pairing);
	element_pow_zn(R2, R2, sx);

	pairing_apply(zt, v, gpk->omega, pairing);
	element_neg(e0, salpha);
	element_pow_zn(zt, zt, e0);
	element_mul(R2, R2, zt);


	pairing_apply(zt, v, gpk->g2, pairing);
	element_neg(e0, sdelta);
	element_pow_zn(zt, zt, e0);
	element_mul(R2, R2, zt);

	pairing_apply(zt, T2, gpk->omega, pairing);
	pairing_apply(zt2, gpk->g1, gpk->g2, pairing);

	element_div(zt, zt, zt2);
	element_pow_zn(zt, zt, c);
	element_mul(R2, R2, zt);

	unsigned char digest[hash_length];

	hash_input_length = element_length_in_bytes(gpk->g1) +
	element_length_in_bytes(gpk->g2) +
	element_length_in_bytes(gpk->omega) +
	element_length_in_bytes(M) +
	element_length_in_bytes(r) +
	element_length_in_bytes(T1) +
	element_length_in_bytes(T2) +
	element_length_in_bytes(R1) +
	element_length_in_bytes(R2) +
	element_length_in_bytes(R3);

	hash_input = malloc(hash_input_length);

	hash_input += element_to_bytes(hash_input, gpk->g1);
	hash_input += element_to_bytes(hash_input, gpk->g2);
	hash_input += element_to_bytes(hash_input, gpk->omega);
	hash_input += element_to_bytes(hash_input, M); // Could avoid converting to bytes and from bytes
	hash_input += element_to_bytes(hash_input, r);
	hash_input += element_to_bytes(hash_input, T1);
	hash_input += element_to_bytes(hash_input, T2);
	hash_input += element_to_bytes(hash_input, R1);
	hash_input += element_to_bytes(hash_input, R2);
	hash_input += element_to_bytes(hash_input, R3);
	hash_input -= hash_input_length;

	hash_init(context);
	hash_update(context, hash_input, hash_input_length);
	hash_final(digest, context);
	free(hash_input);

	element_from_hash(c2, digest, sizeof(digest));

	int result = 0;
	if (!element_cmp(c, c2)) {
		result = 1;
	}

	element_t u2, v2;

	element_init_G2(u2, pairing);
	element_init_G2(v2, pairing);

	element_from_hash(u2, digestUV, sizeof(digestUV));
	element_from_hash(v2, digestUV, sizeof(digestUV));

	pairing_apply(zt, T1, v2, pairing);

	int i = 0;
	for(i = 0; i < RLlen; i++)
	{
		element_div(z0, T2, RL[i]->A);
		pairing_apply(zt2, z0, u2, pairing);

        if(element_cmp(zt, zt2) == 0){
			result = -1; //Revoqued
			break;
		}
	}

element_clear(u2);
element_clear(v2);



	#ifdef DEBUG
	element_printf("T1: %B\n", T1);
	element_printf("T2: %B\n", T2);
	element_printf("R1: %B\n", R1);
	element_printf("R2: %B\n", R2);
	element_printf("R3: %B\n", R3);
	element_printf("r: %B\n", r);
	element_printf("M: %B\n\n", M);
	element_printf("c: %B\n", c);
	element_printf("c2: %B\n", c2);


	element_printf("ralpha: %B\n", salpha);
	element_printf("rx: %B\n", sx);
	element_printf("rdelta: %B\n", sdelta);
	#endif

	element_clear(u);
	element_clear(v);


	element_clear(M);
	element_clear(c);
	element_clear(c2);
	element_clear(r);

	element_clear(T1);
	element_clear(T2);
	element_clear(R1);
	element_clear(R2);
	element_clear(R3);

	element_clear(salpha);
	element_clear(sx);
	element_clear(sdelta);

	element_clear(e0);
	element_clear(zt);
	element_clear(zt2);
	element_clear(z0);
	return result;
}

int vlr_revoc(int i, int n, vlr_group_revocation_token_t *grt, vlr_revocation_list_t *RL, int *RLlen)
{
	if(i >= n) return 0; //User doesn't exists
	if(*RLlen >= n) return -1; //RL too long

	memcpy(RL[*RLlen], grt[i], sizeof(grt));
	(*RLlen)++;
	return 1;
}

void vlr_free_usk(vlr_user_private_key_t *usk, int n)
{
	int i = 0;
	for (i = 0; i < n; i++)
	{
		element_clear(usk[i]->A);
		element_clear(usk[i]->x);
	}
}

void vlr_free_grt(vlr_group_revocation_token_t *grt, int n)
{
	int i = 0;
	for (i = 0; i < n; i++)
	{
		element_clear(grt[i]->A);
	}
}

void vlr_free_gpk(vlr_group_public_key_t gpk)
{
		element_clear(gpk->g1);
		element_clear(gpk->g2);
		element_clear(gpk->omega);
}
