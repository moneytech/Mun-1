#include <mun/location.h>
#include <mun/bitfield.h>

enum{
  kBitsForKind = 4,
  kBitsForPayload = sizeof(word) * 8 - kBitsForKind
};

static const uword kLocationMask = 0x3;

static const bit_field kPayloadField = {
    kBitsForKind,
    kBitsForPayload
};

static const bit_field kKindField = {
    0,
    kBitsForKind
};

static const bit_field kPolicyField = {
    0, 3
};

location_kind
loc_get_kind(location loc){
  return ((location_kind) bit_field_decode(&kKindField, loc));
}

location_policy
loc_get_policy(location loc){
  return ((location_policy) bit_field_decode(&kPolicyField, loc_get_payload(loc)));
}

uword
loc_get_payload(location loc){
  return ((uword) bit_field_decode(&kPayloadField, loc));
}

void
loc_init(location* loc){
  *loc = kInvalidLocation;
}

MUN_INLINE uword
unallocated(location_policy policy){
  uword payload = ((uword) bit_field_encode(&kPolicyField, policy));
  return ((uword) bit_field_encode(&kKindField, kUnallocated) | bit_field_encode(&kPayloadField, payload));
}

MUN_INLINE uword
allocated(location_kind kind, uword payload){
  return ((uword) bit_field_encode(&kKindField, kind) | bit_field_encode(&kPayloadField, payload));
}

void
loc_init_s(location* loc){
  *loc = unallocated(kSameAsFirstInput);
}

void
loc_init_a(location* loc){
  *loc = unallocated(kAny);
}

void
loc_init_r(location* loc, asm_register reg){
  *loc = allocated(kRegister, reg);
}

void
loc_init_x(location* loc, asm_fpu_register reg){
  *loc = allocated(kFpuRegister, reg);
}

void
loc_init_c(location* loc, constant_instr* obj){
  *loc = ((uword) obj) | kConstant;
}

bool
loc_is_constant(location loc){
  return (loc & kLocationMask) == kConstant;
}

bool
loc_is_invalid(location loc){
  return loc == kInvalidLocation;
}

void
loc_hint_pr(location* loc){
  *loc = unallocated(kPrefersRegister);
}

void
loc_hint_rr(location* loc){
  *loc = unallocated(kRequiresRegister);
}

void
loc_hint_rx(location* loc){
  *loc = unallocated(kRequiresFpuRegister);
}

constant_instr*
loc_get_constant(location loc){
  return ((constant_instr*) (loc & ~kLocationMask));
}

location_summary*
loc_summary_new(word in_count, word temp_count, bool call){
  location_summary* summary = malloc(sizeof(location_summary));
  summary->inputs = malloc(sizeof(location) * in_count);
  summary->temps = malloc(sizeof(location) * temp_count);
  summary->contains_call = call;
  return summary;
}

location_summary*
loc_summary_make(word in_count, location out, bool call){
  location_summary* summary = loc_summary_new(in_count, 0, call);
  for(word i = 0; i < in_count; i++) {
    loc_hint_rr(&summary->inputs[i]);
  }
  summary->output = out;
  return summary;
}