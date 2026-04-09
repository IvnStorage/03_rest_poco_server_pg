#!/bin/bash

set -euo pipefail

BASE_URL="${1:-http://localhost:8080}"

TEST_LOGIN="reader_auto_$(date +%s)"
TEST_PASSWORD="secret123"
TEST_FIRST_NAME="Ivan"
TEST_LAST_NAME="Petrov"

USER2_LOGIN="reader2_auto_$(date +%s)"
USER2_PASSWORD="pass456"
USER2_FIRST_NAME="Anna"
USER2_LAST_NAME="Smirnova"

BOOK_TITLE="War and Peace"
BOOK_AUTHOR="Leo Tolstoy"
BOOK_YEAR=1869

ISSUED_AT="2026-03-28"
RETURNED_AT="2026-04-05"

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "$TMP_DIR"' EXIT

LAST_STATUS=""
LAST_BODY=""
LAST_METHOD=""
LAST_URL=""
LAST_REQUEST_BODY=""
LAST_HAS_TOKEN="no"

print_step() {
  echo ""
  echo "=================================================="
  echo "==> $1"
  echo "=================================================="
}

fail() {
  echo ""
  echo "TEST FAILED: $1"
  echo "----------------------------------------"
  echo "Request:"
  echo "  Method: ${LAST_METHOD:-}"
  echo "  URL:    ${LAST_URL:-}"
  echo "  Token:  ${LAST_HAS_TOKEN:-no}"
  if [[ -n "${LAST_REQUEST_BODY:-}" ]]; then
    echo "  Body:   ${LAST_REQUEST_BODY}"
  else
    echo "  Body:   <empty>"
  fi
  echo ""
  echo "Response:"
  echo "  Status: ${LAST_STATUS:-}"
  if [[ -n "${LAST_BODY:-}" && -f "${LAST_BODY:-}" ]]; then
    echo "  Body:"
    cat "$LAST_BODY"
    echo ""
  else
    echo "  Body: <not available>"
  fi
  exit 1
}

pretty_json_file() {
  local file="$1"

  local win_file
  win_file="$(cygpath -w "$file")"

  powershell.exe -NoProfile -Command "
    try {
      \$raw = Get-Content -Raw -Path '$win_file'
      if ([string]::IsNullOrWhiteSpace(\$raw)) {
        Write-Output '<empty>'
      } else {
        \$obj = \$raw | ConvertFrom-Json
        \$obj | ConvertTo-Json -Depth 20
      }
    } catch {
      Get-Content -Raw -Path '$win_file'
    }
  " | tr -d '\r'
}

pretty_json_text() {
  local text="$1"
  if [[ -z "$text" ]]; then
    echo "<empty>"
    return
  fi

  local tmp_file="$TMP_DIR/request_pretty.json"
  printf '%s' "$text" > "$tmp_file"
  pretty_json_file "$tmp_file"
}

json_get() {
  local file="$1"
  local expr="$2"

  local win_file
  win_file="$(cygpath -w "$file")"

  powershell.exe -NoProfile -Command "
    \$data = Get-Content -Raw -Path '$win_file' | ConvertFrom-Json;
    \$value = \$data;
    foreach (\$part in '$expr'.Split('.')) {
      if (\$part -match '^\d+$') {
        \$value = \$value[[int]\$part];
      } else {
        \$value = \$value.\$part;
      }
    }
    if (\$null -eq \$value) {
      Write-Output ''
    } elseif (\$value -is [bool]) {
      if (\$value) { Write-Output 'true' } else { Write-Output 'false' }
    } else {
      Write-Output \$value
    }
  " | tr -d '\r'
}

print_request_response() {
  echo "Request:"
  echo "  Method: $LAST_METHOD"
  echo "  URL:    $LAST_URL"
  echo "  Token:  $LAST_HAS_TOKEN"
  if [[ -n "$LAST_REQUEST_BODY" ]]; then
    echo "  Body:"
    pretty_json_text "$LAST_REQUEST_BODY" | sed 's/^/    /'
  else
    echo "  Body: <empty>"
  fi

  echo ""
  echo "Response:"
  echo "  Status: $LAST_STATUS"
  echo "  Body:"
  pretty_json_file "$LAST_BODY" | sed 's/^/    /'
  echo ""
}

request() {
  local method="$1"
  local url="$2"
  local body="${3:-}"
  local auth_token="${4:-}"

  local body_file="$TMP_DIR/response_body.json"
  local headers_file="$TMP_DIR/response_headers.txt"

  rm -f "$body_file" "$headers_file"

  LAST_METHOD="$method"
  LAST_URL="$url"
  LAST_REQUEST_BODY="$body"

  if [[ -n "$auth_token" ]]; then
    LAST_HAS_TOKEN="yes"
  else
    LAST_HAS_TOKEN="no"
  fi

  local curl_args=(
    -sS
    -X "$method"
    -D "$headers_file"
    -o "$body_file"
    "$url"
  )

  if [[ -n "$body" ]]; then
    curl_args+=(-H "Content-Type: application/json" --data "$body")
  fi

  if [[ -n "$auth_token" ]]; then
    curl_args+=(-H "Authorization: Bearer $auth_token")
  fi

  curl "${curl_args[@]}"

  LAST_BODY="$body_file"
  LAST_STATUS="$(awk 'NR==1 {print $2}' "$headers_file")"

  print_request_response
}

assert_status() {
  local expected="$1"
  if [[ "$LAST_STATUS" != "$expected" ]]; then
    fail "Expected HTTP $expected, got $LAST_STATUS"
  fi
}

assert_json_equals() {
  local file="$1"
  local expr="$2"
  local expected="$3"

  local actual
  actual="$(json_get "$file" "$expr")"

  if [[ "$actual" != "$expected" ]]; then
    fail "Expected JSON field '$expr' to be '$expected', got '$actual'"
  fi
}

assert_json_nonempty() {
  local file="$1"
  local expr="$2"

  local actual
  actual="$(json_get "$file" "$expr")"

  if [[ -z "$actual" ]]; then
    fail "Expected JSON field '$expr' to be non-empty"
  fi
}

print_step "1. Register first user"
request "POST" "$BASE_URL/api/v1/auth/register" \
  "{\"login\":\"$TEST_LOGIN\",\"password\":\"$TEST_PASSWORD\",\"firstName\":\"$TEST_FIRST_NAME\",\"lastName\":\"$TEST_LAST_NAME\"}"
assert_status "201"
assert_json_equals "$LAST_BODY" "login" "$TEST_LOGIN"
assert_json_equals "$LAST_BODY" "firstName" "$TEST_FIRST_NAME"
assert_json_nonempty "$LAST_BODY" "id"

USER1_ID="$(json_get "$LAST_BODY" "id")"

print_step "2. Register duplicate user should fail"
request "POST" "$BASE_URL/api/v1/auth/register" \
  "{\"login\":\"$TEST_LOGIN\",\"password\":\"$TEST_PASSWORD\",\"firstName\":\"$TEST_FIRST_NAME\",\"lastName\":\"$TEST_LAST_NAME\"}"
assert_status "409"

print_step "3. Login first user"
request "POST" "$BASE_URL/api/v1/auth/login" \
  "{\"login\":\"$TEST_LOGIN\",\"password\":\"$TEST_PASSWORD\"}"
assert_status "200"
assert_json_nonempty "$LAST_BODY" "token"

TOKEN="$(json_get "$LAST_BODY" "token")"

print_step "4. Login with wrong password should fail"
request "POST" "$BASE_URL/api/v1/auth/login" \
  "{\"login\":\"$TEST_LOGIN\",\"password\":\"wrong_password\"}"
assert_status "401"

print_step "5. Create second user through /api/v1/users"
request "POST" "$BASE_URL/api/v1/users" \
  "{\"login\":\"$USER2_LOGIN\",\"password\":\"$USER2_PASSWORD\",\"firstName\":\"$USER2_FIRST_NAME\",\"lastName\":\"$USER2_LAST_NAME\"}"
assert_status "201"
assert_json_equals "$LAST_BODY" "login" "$USER2_LOGIN"
USER2_ID="$(json_get "$LAST_BODY" "id")"

print_step "6. Find user by login"
request "GET" "$BASE_URL/api/v1/users/by-login/$USER2_LOGIN"
assert_status "200"
assert_json_equals "$LAST_BODY" "login" "$USER2_LOGIN"
assert_json_equals "$LAST_BODY" "firstName" "$USER2_FIRST_NAME"

print_step "7. Search users by first/last name"
request "GET" "$BASE_URL/api/v1/users/search?firstName=Ann&lastName=Smir"
assert_status "200"
COUNT_USERS="$(json_get "$LAST_BODY" "count")"
if [[ "$COUNT_USERS" -lt 1 ]]; then
  fail "Expected at least one user in search results"
fi

print_step "8. Create book without token should fail"
request "POST" "$BASE_URL/api/v1/books" \
  "{\"title\":\"$BOOK_TITLE\",\"author\":\"$BOOK_AUTHOR\",\"year\":$BOOK_YEAR}"
assert_status "401"

print_step "9. Create book with token"
request "POST" "$BASE_URL/api/v1/books" \
  "{\"title\":\"$BOOK_TITLE\",\"author\":\"$BOOK_AUTHOR\",\"year\":$BOOK_YEAR}" \
  "$TOKEN"
assert_status "201"
assert_json_equals "$LAST_BODY" "title" "$BOOK_TITLE"
assert_json_equals "$LAST_BODY" "author" "$BOOK_AUTHOR"
assert_json_equals "$LAST_BODY" "available" "true"

BOOK_ID="$(json_get "$LAST_BODY" "id")"

print_step "10. Search book by title"
request "GET" "$BASE_URL/api/v1/books/search?title=War"
assert_status "200"
COUNT_BOOKS_TITLE="$(json_get "$LAST_BODY" "count")"
if [[ "$COUNT_BOOKS_TITLE" -lt 1 ]]; then
  fail "Expected at least one book in title search results"
fi

print_step "11. Search book by author"
request "GET" "$BASE_URL/api/v1/books/search?author=Tolstoy"
assert_status "200"
COUNT_BOOKS_AUTHOR="$(json_get "$LAST_BODY" "count")"
if [[ "$COUNT_BOOKS_AUTHOR" -lt 1 ]]; then
  fail "Expected at least one book in author search results"
fi

print_step "12. Create loan without token should fail"
request "POST" "$BASE_URL/api/v1/loans" \
  "{\"userId\":$USER2_ID,\"bookId\":$BOOK_ID,\"issuedAt\":\"$ISSUED_AT\"}"
assert_status "401"

print_step "13. Create loan with token"
request "POST" "$BASE_URL/api/v1/loans" \
  "{\"userId\":$USER2_ID,\"bookId\":$BOOK_ID,\"issuedAt\":\"$ISSUED_AT\"}" \
  "$TOKEN"
assert_status "201"
assert_json_equals "$LAST_BODY" "userId" "$USER2_ID"
assert_json_equals "$LAST_BODY" "bookId" "$BOOK_ID"
assert_json_equals "$LAST_BODY" "returned" "false"

LOAN_ID="$(json_get "$LAST_BODY" "id")"

print_step "14. Create second loan for same book should fail"
request "POST" "$BASE_URL/api/v1/loans" \
  "{\"userId\":$USER1_ID,\"bookId\":$BOOK_ID,\"issuedAt\":\"$ISSUED_AT\"}" \
  "$TOKEN"
assert_status "409"

print_step "15. Get loans of user"
request "GET" "$BASE_URL/api/v1/users/$USER2_ID/loans"
assert_status "200"
COUNT_LOANS="$(json_get "$LAST_BODY" "count")"
if [[ "$COUNT_LOANS" -lt 1 ]]; then
  fail "Expected at least one loan in user loans response"
fi

print_step "16. Return loan without token should fail"
request "PATCH" "$BASE_URL/api/v1/loans/$LOAN_ID/return" \
  "{\"returnedAt\":\"$RETURNED_AT\"}"
assert_status "401"

print_step "17. Return loan with token"
request "PATCH" "$BASE_URL/api/v1/loans/$LOAN_ID/return" \
  "{\"returnedAt\":\"$RETURNED_AT\"}" \
  "$TOKEN"
assert_status "200"
assert_json_equals "$LAST_BODY" "returned" "true"
assert_json_equals "$LAST_BODY" "returnedAt" "$RETURNED_AT"

print_step "18. Return same loan again should fail"
request "PATCH" "$BASE_URL/api/v1/loans/$LOAN_ID/return" \
  "{\"returnedAt\":\"$RETURNED_AT\"}" \
  "$TOKEN"
assert_status "409"

print_step "19. User not found"
request "GET" "$BASE_URL/api/v1/users/by-login/user_that_does_not_exist_12345"
assert_status "404"

print_step "20. Loan not found"
request "PATCH" "$BASE_URL/api/v1/loans/999999/return" \
  "{\"returnedAt\":\"$RETURNED_AT\"}" \
  "$TOKEN"
assert_status "404"

echo ""
echo "=================================================="
echo "ALL TESTS PASSED"
echo "=================================================="